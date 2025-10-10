/**
 * @file app_ble.c
 * @author ZB (2536566200@qq.com)
 * @brief 蓝牙应用
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "app_ble.h"

#define TAG "APP_BLE"

static void _ble_task(void *param);


app_ble_status_e app_ble_init(void)
{
    /**< 1.创建消息队列 */
    

    /**< 2.创建蓝牙线程 */
    xTaskCreate(_ble_task,                                                              /**< 按键线程 */
                APP_BLE_TASK_NAME,                                                      /**< 线程名称 */
                APP_BLE_TASK_STACK,                                                     /**< 线程堆栈大小 */
                APP_BLE_TASK_PARAM,                                                     /**< 用户参数 */
                APP_BLE_TASK_PRIO,                                                      /**< 线程优先级 */
                APP_BLE_TASK_HANDLER);                                                  /**< 线程句柄 */

    return APP_BLE_STATUS_OK;
}

static void _ble_task(void *param)
{
    for (;;) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


