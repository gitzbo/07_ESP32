/**
 * @file main.h
 * @author ZB (2536566200@qq.com)
 * @brief main头文件
 * @version 0.1
 * @date 2025-10-07
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __MAIN_H__
#define __MAIN_H__

/**< 标准库 */
#include <stdio.h>
#include <inttypes.h>

/**< 配置文件 */
#include "sdkconfig.h"

/**< 系统库 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**< esp库 */
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

/**< BSP库 */
#include "bsp_nvs_driver.h"
#include "bsp_key_driver.h"
#include "bsp_ble_driver.h"
#include "bsp_wifi_driver.h"

/**< APP库 */
#include "app_key.h"
#include "app_ble.h"
#include "app_mqtt.h"
#include "app_ota.h"

/**< 开源库 */
#include "shell_port.h"






#endif





