/**
 * @file app_ota.h
 * @author ZB (2536566200@qq.com)
 * @brief OTA应用
 * @version 0.1
 * @date 2025-10-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __APP_OTA_H__
#define __APP_OTA_H__

#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"

#define BUFFSIZE        1024
#define HASH_LEN        32
#define OTA_URL_SIZE    256

#define APP_OTA_TASK_NAME    "APP_OTA"
#define APP_OTA_TASK_STACK   8192
#define APP_OTA_TASK_PARAM   NULL
#define APP_OTA_TASK_PRIO    9
#define APP_OTA_TASK_HANDLER NULL


typedef enum {
    APP_OTA_STATUS_OK = 0,
    APP_OTA_STATUS_ERROR,
}app_ota_status_e;

/**
 * @brief OTA应用初始化
 * 
 * @return app_ota_status_e 运行状态
 */
app_ota_status_e app_ota_init(void);







#endif


