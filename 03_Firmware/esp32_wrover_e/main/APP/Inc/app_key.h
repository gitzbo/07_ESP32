/**
 * @file app_key.h
 * @author ZB (2536566200@qq.com)
 * @brief 按键应用
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __APP_KEY_H__
#define __APP_KEY_H__

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp_key_driver.h"
#include "multi_button.h"

#define APP_KEY_TASK_NAME    "KEY_TASK"
#define APP_KEY_TASK_STACK   1024
#define APP_KEY_TASK_PARAM   NULL
#define APP_KEY_TASK_PRIO    9
#define APP_KEY_TASK_HANDLER NULL

typedef enum {
    APP_KEY_STATUS_OK = 0,
    APP_KEY_STATUS_ERROR,
}app_key_status_e;

/**
 * @brief 按键应用初始化
 * 
 * @return app_key_status_e 运行状态
 */
app_key_status_e app_key_init(void);


#endif

