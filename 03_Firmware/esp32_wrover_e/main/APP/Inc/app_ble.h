/**
 * @file app_ble.h
 * @author ZB (2536566200@qq.com)
 * @brief BLE应用
 * 1.蓝牙发送
 * 2.蓝牙接收
 * 3.蓝牙MESH
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __APP_BLE_H__
#define __APP_BLE_H__

#include <stdio.h>
#include <string.h>
#include <sdkconfig.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define APP_BLE_TASK_NAME    "BLE_TASK"
#define APP_BLE_TASK_STACK   1024
#define APP_BLE_TASK_PARAM   NULL
#define APP_BLE_TASK_PRIO    9
#define APP_BLE_TASK_HANDLER NULL

typedef enum {
    APP_BLE_STATUS_OK = 0,
    APP_BLE_STATUS_ERROR,
}app_ble_status_e;

/**
 * @brief 初始化app应用
 * 
 * @return app_ble_status_e 运行状态
 */
app_ble_status_e app_ble_init(void);




#endif
