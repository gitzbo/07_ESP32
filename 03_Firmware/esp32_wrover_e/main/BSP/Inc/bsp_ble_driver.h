/**
 * @file bsp_ble_driver.h
 * @author ZB (2536566200@qq.com)
 * @brief 蓝牙驱动
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __BSP_BLE_DRIVER_H__
#define __BSP_BLE_DRIVER_H__

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"



// #include "ble_mesh_example_init.h"

typedef enum {
    BSP_BLE_DRIVER_STATUS_OK = 0,
    BSP_BLE_DRIVER_STATUS_ERROR,
}bsp_ble_driver_status_e;

/**
 * @brief 蓝牙驱动初始化
 * 
 * @return bsp_ble_driver_status_e 运行状态
 */
bsp_ble_driver_status_e bsp_ble_driver_init(void);


#endif

