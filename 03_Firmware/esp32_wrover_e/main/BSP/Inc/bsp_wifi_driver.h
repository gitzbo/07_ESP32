/**
 * @file bsp_wifi_driver.h
 * @author ZB (2536566200@qq.com)
 * @brief WIFI驱动
 * @version 0.1
 * @date 2025-10-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __BSP_WIFI_DRIVER_H__
#define __BSP_WIFI_DRIVER_H__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "protocol_examples_common.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"

typedef enum {
    BSP_WIFI_DRIVER_OK = 0,
    BSP_WIFI_DRIVER_ERROR
}bsp_wifi_driver_status_e;

/**
 * @brief wifi驱动初始化
 * 
 * @return bsp_wifi_driver_status_e 运行状态
 */
bsp_wifi_driver_status_e bsp_wifi_driver_init(void);






#endif




