/*
 * 运行时诊断任务文件。
 * 本文件周期性采集 FreeRTOS 任务状态、堆剩余量和任务栈高水位，
 * 主要用于定位“卡死、爆栈、堆不足”这类系统级问题。
 */
#define LOG_TAG "task_diag"
#define LOG_LVL ELOG_LVL_INFO

#include "FreeRTOS.h"
#include "portable.h"
#include "task.h"

#include "elog.h"

#include <stdint.h>
#include <string.h>

#define TASK_DIAG_MONITOR_PERIOD_MS 5000U
#define TASK_DIAG_MAX_TASKS         16U

#define TASK_DIAG_WARN_STACK_WORDS 96U
#define TASK_DIAG_WARN_HEAP_BYTES  2048U

#define TASK_DIAG_FLAG_STACK_LOW    (1UL << 0)
#define TASK_DIAG_FLAG_HEAP_LOW     (1UL << 1)
#define TASK_DIAG_FLAG_CAPTURE_FAIL (1UL << 2)
#define TASK_DIAG_FLAG_TASK_TRUNC   (1UL << 3)

typedef struct
{
    char name[configMAX_TASK_NAME_LEN];
    uint32_t task_number;
    uint32_t current_priority;
    uint32_t state;
    uint32_t stack_high_water_words;
} task_diag_entry_t;

typedef struct
{
    uint32_t heartbeat;
    uint32_t sample_seq;
    uint32_t last_sample_tick;
    uint32_t warning_flags;
    uint32_t task_count;
    uint32_t task_capacity;
    uint32_t captured_task_count;
    uint32_t free_heap_bytes;
    uint32_t min_ever_free_heap_bytes;
    uint32_t largest_free_block_bytes;
    uint32_t smallest_free_block_bytes;
    uint32_t free_block_count;
    uint32_t successful_allocations;
    uint32_t successful_frees;
    uint32_t worst_stack_words;
    uint32_t worst_task_index;
    task_diag_entry_t tasks[TASK_DIAG_MAX_TASKS];
} task_diag_snapshot_t;

/* Visible in Ozone to identify which task is closest to overflowing. */
volatile task_diag_snapshot_t g_task_diag_snapshot;

static TaskStatus_t s_task_diag_status[TASK_DIAG_MAX_TASKS];
static uint32_t s_last_warning_flags = 0U;

/* 将任务名安全地复制到诊断结构中，避免越界。 */
static void task_diag_copy_name(volatile char *dst, uint32_t dst_size, const char *src)
{
    uint32_t i;

    if ((dst == NULL) || (dst_size == 0U))
    {
        return;
    }

    if (src == NULL)
    {
        dst[0] = '\0';
        return;
    }

    for (i = 0U; (i + 1U) < dst_size; i++)
    {
        char ch = src[i];
        dst[i] = ch;

        if (ch == '\0')
        {
            return;
        }
    }

    dst[dst_size - 1U] = '\0';
}

/* 将 FreeRTOS 任务状态枚举转换为单字符，便于在日志中紧凑显示。 */
static char task_diag_state_to_char(uint32_t state)
{
    switch ((eTaskState)state)
    {
    case eRunning:
        return 'R';

    case eReady:
        return 'Y';

    case eBlocked:
        return 'B';

    case eSuspended:
        return 'S';

    case eDeleted:
        return 'D';

    case eInvalid:
    default:
        return 'I';
    }
}

/*
 * 采集一次系统快照并写入全局诊断结构。
 * Ozone 中可以直接观察 `g_task_diag_snapshot`，快速判断哪个任务最接近爆栈。
 */
static void task_diag_collect_snapshot(void)
{
    HeapStats_t heap_stats;
    UBaseType_t captured_task_count;
    uint32_t i;
    uint32_t warning_flags = 0U;
    uint32_t worst_stack_words = 0xFFFFFFFFUL;
    uint32_t worst_task_index = 0U;

    (void)memset(&heap_stats, 0, sizeof(heap_stats));
    (void)memset(s_task_diag_status, 0, sizeof(s_task_diag_status));

    vPortGetHeapStats(&heap_stats);
    captured_task_count = uxTaskGetSystemState(s_task_diag_status, TASK_DIAG_MAX_TASKS, NULL);

    g_task_diag_snapshot.heartbeat++;
    g_task_diag_snapshot.sample_seq++;
    g_task_diag_snapshot.last_sample_tick = (uint32_t)xTaskGetTickCount();
    g_task_diag_snapshot.task_count = (uint32_t)uxTaskGetNumberOfTasks();
    g_task_diag_snapshot.task_capacity = TASK_DIAG_MAX_TASKS;
    g_task_diag_snapshot.captured_task_count = (uint32_t)captured_task_count;
    g_task_diag_snapshot.free_heap_bytes = (uint32_t)heap_stats.xAvailableHeapSpaceInBytes;
    g_task_diag_snapshot.min_ever_free_heap_bytes = (uint32_t)xPortGetMinimumEverFreeHeapSize();
    g_task_diag_snapshot.largest_free_block_bytes = (uint32_t)heap_stats.xSizeOfLargestFreeBlockInBytes;
    g_task_diag_snapshot.smallest_free_block_bytes = (uint32_t)heap_stats.xSizeOfSmallestFreeBlockInBytes;
    g_task_diag_snapshot.free_block_count = (uint32_t)heap_stats.xNumberOfFreeBlocks;
    g_task_diag_snapshot.successful_allocations = (uint32_t)heap_stats.xNumberOfSuccessfulAllocations;
    g_task_diag_snapshot.successful_frees = (uint32_t)heap_stats.xNumberOfSuccessfulFrees;

    if (g_task_diag_snapshot.task_count > TASK_DIAG_MAX_TASKS)
    {
        warning_flags |= TASK_DIAG_FLAG_TASK_TRUNC;
    }

    if (captured_task_count == 0U)
    {
        warning_flags |= TASK_DIAG_FLAG_CAPTURE_FAIL;
    }

    if (g_task_diag_snapshot.min_ever_free_heap_bytes < TASK_DIAG_WARN_HEAP_BYTES)
    {
        warning_flags |= TASK_DIAG_FLAG_HEAP_LOW;
    }

    for (i = 0U; i < TASK_DIAG_MAX_TASKS; i++)
    {
        g_task_diag_snapshot.tasks[i].name[0] = '\0';
        g_task_diag_snapshot.tasks[i].task_number = 0U;
        g_task_diag_snapshot.tasks[i].current_priority = 0U;
        g_task_diag_snapshot.tasks[i].state = (uint32_t)eInvalid;
        g_task_diag_snapshot.tasks[i].stack_high_water_words = 0U;
    }

    for (i = 0U; i < (uint32_t)captured_task_count; i++)
    {
        uint32_t stack_words = (uint32_t)s_task_diag_status[i].usStackHighWaterMark;

        task_diag_copy_name(g_task_diag_snapshot.tasks[i].name,
                            (uint32_t)sizeof(g_task_diag_snapshot.tasks[i].name),
                            s_task_diag_status[i].pcTaskName);
        g_task_diag_snapshot.tasks[i].task_number = (uint32_t)s_task_diag_status[i].xTaskNumber;
        g_task_diag_snapshot.tasks[i].current_priority = (uint32_t)s_task_diag_status[i].uxCurrentPriority;
        g_task_diag_snapshot.tasks[i].state = (uint32_t)s_task_diag_status[i].eCurrentState;
        g_task_diag_snapshot.tasks[i].stack_high_water_words = stack_words;

        if (stack_words < worst_stack_words)
        {
            worst_stack_words = stack_words;
            worst_task_index = i;
        }
    }

    if (captured_task_count == 0U)
    {
        worst_stack_words = 0U;
        worst_task_index = 0U;
    }

    g_task_diag_snapshot.worst_stack_words = worst_stack_words;
    g_task_diag_snapshot.worst_task_index = worst_task_index;

    if ((captured_task_count > 0U) && (worst_stack_words < TASK_DIAG_WARN_STACK_WORDS))
    {
        warning_flags |= TASK_DIAG_FLAG_STACK_LOW;
    }

    g_task_diag_snapshot.warning_flags = warning_flags;
}

/* 将本次采集结果输出为一条摘要日志，必要时补充每个任务的详细状态。 */
static void task_diag_log_snapshot(void)
{
    uint32_t captured_task_count = g_task_diag_snapshot.captured_task_count;
    uint32_t worst_task_index = g_task_diag_snapshot.worst_task_index;
    const char *worst_task_name = "n/a";
    uint32_t worst_task_stack = 0U;
    uint32_t flags = g_task_diag_snapshot.warning_flags;

    if ((captured_task_count > 0U) && (worst_task_index < captured_task_count))
    {
        worst_task_name = (const char *)g_task_diag_snapshot.tasks[worst_task_index].name;
        worst_task_stack = g_task_diag_snapshot.tasks[worst_task_index].stack_high_water_words;
    }

    log_i("diag#%lu hb=%lu heap=%lu min=%lu biggest=%lu blocks=%lu worst=%s:%luw flags=0x%02lx tasks=%lu/%lu",
          (unsigned long)g_task_diag_snapshot.sample_seq,
          (unsigned long)g_task_diag_snapshot.heartbeat,
          (unsigned long)g_task_diag_snapshot.free_heap_bytes,
          (unsigned long)g_task_diag_snapshot.min_ever_free_heap_bytes,
          (unsigned long)g_task_diag_snapshot.largest_free_block_bytes,
          (unsigned long)g_task_diag_snapshot.free_block_count,
          worst_task_name,
          (unsigned long)worst_task_stack,
          (unsigned long)flags,
          (unsigned long)captured_task_count,
          (unsigned long)g_task_diag_snapshot.task_count);

    if ((flags != 0U) && (flags != s_last_warning_flags))
    {
        uint32_t i;

        for (i = 0U; i < captured_task_count; i++)
        {
            log_w("diag task[%lu] %s state=%c prio=%lu stack=%luw",
                  (unsigned long)i,
                  (const char *)g_task_diag_snapshot.tasks[i].name,
                  task_diag_state_to_char(g_task_diag_snapshot.tasks[i].state),
                  (unsigned long)g_task_diag_snapshot.tasks[i].current_priority,
                  (unsigned long)g_task_diag_snapshot.tasks[i].stack_high_water_words);
        }
    }

    s_last_warning_flags = flags;
}

/* 周期性诊断任务入口。 */
void task_elog(void *p)
{
    (void)p;

    log_i("task_elog start");

    while (1)
    {
        task_diag_collect_snapshot();
        task_diag_log_snapshot();
        vTaskDelay(pdMS_TO_TICKS(TASK_DIAG_MONITOR_PERIOD_MS));
    }
}
