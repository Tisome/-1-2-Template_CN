// #include "lvgl.h"
// #include "lv_port_indev_template.h"

// volatile uint32_t lvgl_last_key = 0;

// lv_indev_t *indev_keypad;

// static void keypad_read(lv_indev_drv_t *drv,
//                         lv_indev_data_t *data)
// {
//     extern uint32_t lvgl_last_key;

//     if (lvgl_last_key != 0)
//     {
//         data->state = LV_INDEV_STATE_PRESSED;
//         data->key = lvgl_last_key;

//         lvgl_last_key = 0;
//     }
//     else
//     {
//         data->state = LV_INDEV_STATE_RELEASED;
//     }
// }

// void lv_port_indev_init(void)
// {
//     static lv_indev_drv_t indev_drv;

//     lv_indev_drv_init(&indev_drv);

//     indev_drv.type = LV_INDEV_TYPE_KEYPAD;
//     indev_drv.read_cb = keypad_read;

//     indev_keypad = lv_indev_drv_register(&indev_drv);
// }
