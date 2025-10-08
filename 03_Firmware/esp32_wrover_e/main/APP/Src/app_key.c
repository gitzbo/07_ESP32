/**
 * @file app_key.c
 * @author ZB (2536566200@qq.com)
 * @brief 按键应用
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "app_key.h"

static const char *TAG = "KEY";

static Button btn1 = {0};                           /**< 按键对象 */

static void _key_task(void *param);

void IRAM_ATTR key_isr_handler(void *arg)
{
    /**< 外部中断函数 */
}

static void _key_scan_handler_cb(Button* btn)
{
    if(btn->button_id == BSP_KEY_DRIVER_ID_USR) {
        switch(btn->event) {
            case BTN_SINGLE_CLICK:
                ESP_LOGI(TAG, "BTN SINGLE CLICK!");
                break;
            case BTN_DOUBLE_CLICK:
                ESP_LOGI(TAG, "BTN DOUBLE CLICK!");
                break;
            case BTN_LONG_PRESS_START:
                ESP_LOGI(TAG, "BTN LONG PRESS START!");
                break;
            case BTN_LONG_PRESS_HOLD:
                ESP_LOGI(TAG, "BTN LONG PRESS HOLD!");
                break;
            default:
                break;
        }
    }
}

app_key_status_e app_key_init(void)
{
    /**< 1.将按键注册到扫描函数中 */
    button_init(&btn1, bsp_key_driver_get_status, 0, BSP_KEY_DRIVER_ID_USR);
    button_attach(&btn1, BTN_SINGLE_CLICK,      _key_scan_handler_cb);
    button_attach(&btn1, BTN_DOUBLE_CLICK,      _key_scan_handler_cb);
    button_attach(&btn1, BTN_LONG_PRESS_START,  _key_scan_handler_cb);
    button_attach(&btn1, BTN_LONG_PRESS_HOLD,   _key_scan_handler_cb);
    button_start(&btn1);
    
    /**< 2.创建按键线程 */
    xTaskCreate(_key_task,  
                APP_KEY_TASK_NAME, 
                APP_KEY_TASK_STACK,       
                APP_KEY_TASK_PARAM,       
                APP_KEY_TASK_PRIO,          
                APP_KEY_TASK_HANDLER);

    return APP_KEY_STATUS_OK;
}

static void _key_task(void *param)
{
    while(1) {
        button_ticks();             /**< 按键扫描函数*/
        vTaskDelay(3);              /**< 延时3ms */
    }
}






