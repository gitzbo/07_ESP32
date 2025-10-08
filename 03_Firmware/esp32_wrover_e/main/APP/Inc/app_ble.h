/**
 * @file app_ble.h
 * @author ZB (2536566200@qq.com)
 * @brief 蓝牙应用
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
