请你基于当前工作区中的 MCU 工程与 FPGA 工程代码，专门针对“FPGA 先完成前级采样、相关运算和时间特征提取，MCU 再实时计算流速与流量”这一条核心功能主链路，做一次尽可能详细、系统、适合毕业设计论文写作的分析说明，并最终写入：

`./doc/core_flow_explain.md`

这份文档的目标不是简单罗列文件名和函数名，而是形成一份“核心测量功能运作流程与代码实现分析文档”。后续我会将它作为毕业设计论文中“系统工作原理”“软件设计与实现”“关键算法流程”“任务协同机制”“数据流分析”等章节的重要参考材料。因此请你写得正式、清晰、有条理，并尽量把系统是如何真实运行起来的讲透。

========================
一、分析范围
========================

请重点阅读并分析以下目录与文件，并结合它们之间的调用关系展开：

1. FPGA 核心代码
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/Top.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/RUF.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/pwm.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/AD.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/algorithm.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/timeget.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/corr.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/DeltaT.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/comm_spi_slave.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/communication.v`
- `FPGA_prj_code/RUF_MIX.srcs/sources_1/new/SPI8370.v`

2. MCU 启动与中断入口
- `-1-2-Template_CN/User/main.c`
- `-1-2-Template_CN/User/main.h`
- `-1-2-Template_CN/User/gd32f30x_it.c`
- `-1-2-Template_CN/User/gd32f30x_it.h`

3. MCU 任务层
- `-1-2-Template_CN/TestTask/src/does_it_work.c`
- `-1-2-Template_CN/TestTask/src/task_algorithm.c`
- `-1-2-Template_CN/TestTask/src/task_fake_data.c`
- `-1-2-Template_CN/Task/src/task_spi_rx.c`

4. MCU 业务与算法层
- `-1-2-Template_CN/App/src/algorithm_packet.c`
- `-1-2-Template_CN/App/src/algorithm_process.c`
- `-1-2-Template_CN/App/src/algorithm_flow.c`
- `-1-2-Template_CN/App/src/data.c`
- `-1-2-Template_CN/App/src/fake_data.c`
- `-1-2-Template_CN/App/src/freertos_resources.c`
- `-1-2-Template_CN/App/inc/*.h`

5. MCU 外设驱动层
- `-1-2-Template_CN/periph/src/spi.c`
- `-1-2-Template_CN/periph/src/gpio.c`
- `-1-2-Template_CN/periph/src/usart.c`
- `-1-2-Template_CN/periph/src/periph_link.c`
- `-1-2-Template_CN/periph/inc/*.h`

6. 如有必要，可结合以下内容辅助分析
- `-1-2-Template_CN/Middlewares/FreeRTOS/*`
- `-1-2-Template_CN/doc/interface.md`
- `-1-2-Template_CN/doc/modbus_explain.md`
- `-1-2-Template_CN/doc/menus_explain.md`

========================
二、必须特别注意的背景
========================

1. 这个项目本质上是“基于 FPGA 的流量计”：
- FPGA 侧负责激励、采样、前级相关运算、时间特征提取，以及与 MCU 的 SPI 数据交互准备。
- MCU 侧负责参数管理、原始包解析、时间差换算、流速/流量计算、滤波、零漂处理、报警、显示与对外通信。

2. 当前代码中同时存在两条输入链路，请务必区分：
- 真实链路：FPGA 通过 `FPGA_INT + SPI` 把预处理后的原始包交给 MCU。
- 联调链路：`task_fake_data` / `fake_data.c` 直接构造与真实包格式一致的模拟数据，供 MCU 算法链路联调。

3. 请不要把测试链路误判为最终业务主链路。你需要明确区分：
- 理论/目标主链路
- 当前工程中实际启用的运行链路
- 为联调和论文演示而存在的辅助链路

4. 文档用途是毕业设计写作，因此除了说明“代码做了什么”，还要说明：
- 为什么这样分层
- 为什么这样划分 FPGA 与 MCU 职责
- 为什么用 FreeRTOS 任务、中断、队列和任务通知
- 这样设计对实时性、可维护性、联调效率、系统完整性有什么意义

========================
三、核心分析目标
========================

请围绕以下几个目标展开，且务必详细分析：

1. 先说明整个核心功能的总体工作原理
要求讲清楚：
- 从超声激励、回波采样、相关运算、时间特征提取，到 MCU 侧计算流速和流量的完整思路
- FPGA 做到哪一步，MCU 从哪一步接手
- 为什么采用这种“FPGA 前处理 + MCU 后处理”的分工方式

2. 说明 FPGA 侧的核心模块结构
要求讲清楚：
- `Top.v` 与 `RUF.v` 各自扮演什么角色
- `pwm`、`AD`、`algorithm`、`timeget`、`corr`、`DeltaT`、`comm_spi_slave`、`communication` 分别负责什么
- FPGA 内部数据是如何一步步流动的
- 最终发给 MCU 的数据包中包含什么信息，为什么这样组织

3. 说明 MCU 侧从上电到核心功能跑起来的过程
必须尽量按真实执行顺序分析，而不是按目录顺序堆代码：
- 程序入口在哪里
- 硬件初始化做了什么
- FPGA 中断与 SPI 接收链路如何建立
- FreeRTOS 资源如何创建
- 哪些任务被创建
- 当前工程里到底是 `task_spi_rx` 还是 `task_fake_data` 在给 `task_algorithm` 喂数据
- `task_algorithm` 如何成为核心数据消费者

4. 详细梳理 MCU 核心任务
重点关注：
- `task_algorithm`
- `task_spi_rx`
- `task_fake_data`
- `does_it_work()` 中的任务创建关系

对每个关键任务，请分别说明：
- 任务创建位置
- 入口函数
- 优先级、栈大小
- while(1) 主循环逻辑
- 是阻塞等待、通知唤醒、周期执行还是轮询
- 依赖哪些队列、通知或全局状态
- 在核心测量链路中起什么作用

5. 详细梳理关键调用链
请重点写清以下链路，且不要只列函数名，要写成“从触发到结果”的过程：

- 链路A：真实 FPGA 数据进入 MCU 的链路  
  `FPGA_INT -> EXTI -> task_spi_rx -> SPI读包 -> xQueue_Rx_Index_Buf -> task_algorithm`

- 链路B：联调假数据链路  
  `task_fake_data -> fake_data_make_packet -> xQueue_Rx_Index_Buf -> task_algorithm`

- 链路C：算法主处理链路  
  `task_algorithm -> rufx_unpack_packet -> rufx_calc_t1_t2_dt -> algorithm_process_group -> update_flow_outputs`

- 链路D：参数影响测量结果的链路  
  `g_parameters` 中的内径、壁厚、声程角度参数、零漂参数、报警参数等如何影响最终输出

- 链路E：测量结果输出链路  
  `g_algo_out -> GUI / Modbus / 其他上层显示或通信模块`

6. 详细分析数据流
请单独成节说明：
- FPGA 侧的数据流
- MCU 侧的数据流
- FPGA 与 MCU 之间的数据接口
- 原始包格式与字段含义
- `idx_a / idx_b / y1 / y2 / y3 / t1 / t2 / dt / flow_speed / flow_rate` 之间的演化关系

请特别解释：
- 为什么 FPGA 给 MCU 的不是“最终流量值”，而是“初步处理后的特征数据包”
- MCU 为什么还要做抛物线插值、流速换算、滤波、零漂补偿、累计量更新

7. 分析 MCU 算法层各文件分工
请重点说明：
- `algorithm_packet.c` 负责什么
- `algorithm_process.c` 负责什么
- `algorithm_flow.c` 负责什么
- `data.c` 在参数、状态、输出上的角色是什么
- 这些文件之间如何协同

8. 分析实时性与工程设计意义
请从毕业设计角度分析：
- 哪些地方体现了实时性设计
- 哪些地方体现了任务与中断分工
- 哪些地方体现了模块化分层
- 为什么 FPGA 适合做前级高速并行处理
- 为什么 MCU 适合做参数化、可配置、可显示、可通信的后级处理

========================
四、输出风格要求
========================

1. 必须写成长文档，内容充实，不要简略带过。
2. 文风要正式、专业，适合后续直接转化为毕业设计论文表述。
3. 不要只输出项目符号清单，要有成段说明和分析。
4. 尽量结合真实文件名、模块名、函数名、任务名、结构体名、队列名、全局变量名、宏定义进行描述。
5. 对不确定的内容，请明确写“根据代码推断”或“当前代码中尚待进一步联调确认”，不要编造。
6. 重点不是罗列函数，而是讲清：
- 系统工作原理
- 模块分工
- 任务协同
- 数据流
- 控制流
- 关键调用链
- 工程设计意义
7. 允许使用 mermaid 流程图、模块关系图、任务协作图、数据流图增强表达。
8. 请明显区分：
- FPGA 侧工作
- MCU 侧工作
- 当前启用链路
- 真实目标链路与联调替代链路

========================
五、建议文档结构
========================

请参考但不限于以下结构组织文档：

1. 文档目的与分析范围
2. 项目核心功能总体说明
3. FPGA 与 MCU 的功能分工
4. FPGA 侧核心模块结构分析
5. FPGA 侧核心数据处理流程分析
6. MCU 上电启动与核心链路建立过程
7. MCU 关键任务分析
8. FPGA 到 MCU 的数据交互链路分析
9. MCU 算法主链路分析
10. 参数系统与测量结果联动分析
11. 核心数据流与控制流综合分析
12. 当前工程中的真实链路与联调链路区分
13. 系统实时性、模块化与工程价值分析
14. 可直接支撑毕业设计论文写作的内容提炼
15. 结论

========================
六、执行要求
========================

请按以下方式执行：

1. 先通读相关 FPGA 与 MCU 代码
2. 明确当前核心功能主链路与联调替代链路
3. 梳理关键任务、关键调用链和数据流
4. 结合源码生成一份完整、清晰、正式的分析文档
5. 最终直接写入：

`./doc/core_flow_explain.md`

注意：
- 不要只在聊天里给我摘要
- 目标是形成一份可直接用于毕业设计参考的成品文档
- 宁可详细，不要过度压缩
