#include "menu.h"
#include "usart.h"
#include "main.h"
typedef enum {
    OTZZ             = 0,       //1200
    TFZZ             = 1,       //2400
    FEZZ             = 2,       //4800
    NSZZ             = 3,       //9600
    ONTZZ            = 4,       //19200
    TEFZZ            = 5        //38400
} Speed_t;

typedef enum {
    NONE             = 0,       //无校验
    EVEN             = 1,       //偶校验
    ODD              = 2        //奇校验 
} Parity_t;

typedef enum {
    EIGHT_BIT        = 0,       //8位
    NINE_BIT         = 1        //9位
} Word_length_t;

typedef enum {
    ONE              = 0,       //1位
    TWO              = 1        //2位
} Stop_bits_t;
typedef struct {
    Speed_t                    Speed;
    Word_length_t        Word_length;
    Parity_t                  Parity;
    Stop_bits_t            Stop_bits;
} Rs485_t;

Rs485_t Rs485_setting =
{
    .Speed         =   NSZZ,
    .Word_length   =   EIGHT_BIT,
    .Parity        =   NONE,
    .Stop_bits     =   ONE
};


int menu4_speed_handler(void *);
int menu4_word_length_handler(void *);
int menu4_parity_handler(void *);
int menu4_stop_bits_handler(void *);
void Rs485_start();
int menu3_rs485_setting_handler(void * parameter) {
    // 主菜单项
    static const MenuItem main_items[] = {
        {"Baud Rate", menu4_speed_handler, " 波特率 "},
        {"Word Length", menu4_word_length_handler, " 字长 "},
        {"Parity", menu4_parity_handler, " 校验 "},
        {"Stop Bits", menu4_stop_bits_handler, " 停止位 "}
    };
    
    // 主菜单结构
    Menu rs485_setting_menu = {
        .title[0] = "Rs485 Setting",
        .title[1] = " 485设置 ",
        .items_count = sizeof(main_items)/sizeof(main_items[0]),
        .items = main_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&rs485_setting_menu);
}
int Speed_handler(void* parameter)
{
    int pip_index = *(int*)parameter;
    Rs485_setting.Speed = pip_index;
  switch(Rs485_setting.Speed)
  {
      case OTZZ:
            usart_baudrate_set(USART0, 1200);
            break;
      case TFZZ:
            usart_baudrate_set(USART0, 2400);
            break;          
      case FEZZ:
            usart_baudrate_set(USART0, 4800);
            break;
      case NSZZ:
            usart_baudrate_set(USART0, 9600);
            break;
      case ONTZZ:
            usart_baudrate_set(USART0, 19200);
            break;
      case TEFZZ:
            usart_baudrate_set(USART0, 38400);
            break;
  }
    return 0;
}
int menu4_speed_handler(void * parameter) {
     // 材质设置
    static const MenuItem fliuid_items[] = {
        {"1200", Speed_handler},
        {"2400", Speed_handler},
        {"4800", Speed_handler},
        {"9600", Speed_handler},
        {"19200", Speed_handler},
        {"38400", Speed_handler}
    };
    
    // 主菜单结构
    Menu Materialsetting_menu = {
        .title = "Speed",
        .items_count = sizeof(fliuid_items)/sizeof(fliuid_items[0]),
        .items = fliuid_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = Rs485_setting.Speed,
        .type  = 1
    };
    
    return display_menu(&Materialsetting_menu);
}
int word_length_handler(void* parameter)
{
    int pip_index = *(int*)parameter;
    Rs485_setting.Word_length = pip_index;
    switch(Rs485_setting.Word_length)
  {
      case EIGHT_BIT:
            usart_word_length_set(USART0, USART_WL_8BIT);
            break;
      case NINE_BIT:
            usart_word_length_set(USART0, USART_WL_9BIT);
            break;          
  }
    return 0;
}
int menu4_word_length_handler(void * parameter) {
     // 材质设置
    static const MenuItem fliuid_items[] = {
        {"8_BIT", word_length_handler},
        {"9_BIT", word_length_handler}
    };
    
    // 主菜单结构
    Menu Materialsetting_menu = {
        .title = "Word Length",
        .items_count = sizeof(fliuid_items)/sizeof(fliuid_items[0]),
        .items = fliuid_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start =Rs485_setting.Word_length,
        .type  = 1
    };
    
    return display_menu(&Materialsetting_menu);
}
int Parity_handler(void* parameter)
{
    int pip_index = *(int*)parameter;
    Rs485_setting.Parity = pip_index;
      switch(Rs485_setting.Parity)
  {
      case NONE:
	        usart_parity_config(USART0, USART_PM_NONE); //无奇偶校验
            break;
      case EVEN:
	        usart_parity_config(USART0, USART_PM_EVEN); //无奇偶校验
            break;   
      case ODD:
	        usart_parity_config(USART0, USART_PM_ODD); //无奇偶校验
            break;        
  }
    return 0;
}
int menu4_parity_handler(void * parameter) {
     // 材质设置
    static const MenuItem fliuid_items[] = {
        {"NONE", Parity_handler},
        {"EVEN", Parity_handler},
        {"ODD",  Parity_handler}
    };
    
    // 主菜单结构
    Menu Materialsetting_menu = {
        .title = "Parity",
        .items_count = sizeof(fliuid_items)/sizeof(fliuid_items[0]),
        .items = fliuid_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start =Rs485_setting.Parity,
        .type  = 1
    };
    
    return display_menu(&Materialsetting_menu);
}
int Stop_bits_handler(void* parameter)
{
    int pip_index = *(int*)parameter;
    Rs485_setting.Stop_bits = pip_index;
    switch(Rs485_setting.Stop_bits)
    {
        case ONE:
            usart_stop_bit_set(USART0, USART_STB_1BIT);
            break;
        case TWO:
            usart_stop_bit_set(USART0, USART_STB_2BIT);
            break;          
    }
    return 0;
}
int menu4_stop_bits_handler(void * parameter) {
     // 材质设置
    static const MenuItem fliuid_items[] = {
        {"ONE", Stop_bits_handler},
        {"TWO", Stop_bits_handler},
    };
    
    // 主菜单结构
    Menu Materialsetting_menu = {
        .title = "Stop Bits",
        .items_count = sizeof(fliuid_items)/sizeof(fliuid_items[0]),
        .items = fliuid_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start =Rs485_setting.Stop_bits,
        .type  = 1
    };
    
    return display_menu(&Materialsetting_menu);
}
void Rs485_start()
{
    usart_deinit(USART0);
  switch(Rs485_setting.Speed)
  {
      case OTZZ:
            usart_baudrate_set(USART0, 1200);
            break;
      case TFZZ:
            usart_baudrate_set(USART0, 2400);
            break;          
      case FEZZ:
            usart_baudrate_set(USART0, 4800);
            break;
      case NSZZ:
            usart_baudrate_set(USART0, 9600);
            break;
      case ONTZZ:
            usart_baudrate_set(USART0, 19200);
            break;
      case TEFZZ:
            usart_baudrate_set(USART0, 38400);
            break;
  }
    switch(Rs485_setting.Word_length)
  {
      case EIGHT_BIT:
	        usart_word_length_set(USART0, USART_WL_8BIT); //bit长度
            break;
      case NINE_BIT:
	        usart_word_length_set(USART0, USART_WL_9BIT); //bit长度
            break;          
  }
      switch(Rs485_setting.Stop_bits)
  {
      case ONE:
	        usart_stop_bit_set(USART0, USART_STB_1BIT);   //停止位
            break;
      case TWO:
	        usart_stop_bit_set(USART0, USART_STB_2BIT);   //停止位
            break;          
  }
      switch(Rs485_setting.Parity)
  {
      case NONE:
	        usart_parity_config(USART0, USART_PM_NONE); //无奇偶校验
            break;
      case EVEN:
	        usart_parity_config(USART0, USART_PM_EVEN); //无奇偶校验
            break;   
      case ODD:
	        usart_parity_config(USART0, USART_PM_ODD); //无奇偶校验
            break;        
  }

	usart_transmit_config(USART0, USART_TRANSMIT_ENABLE); //使能发送
	usart_receive_config(USART0, USART_RECEIVE_ENABLE);   //使能接收
    usart_enable(USART0);

    /* enable USART idle interrupt */
    usart_interrupt_enable(USART0, USART_INT_IDLE);
	nvic_irq_enable(USART0_IRQn, 5, 0);
    
    usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);
}