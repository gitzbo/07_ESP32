/**
 * @file bsp_wifi_driver.c
 * @author ZB (2536566200@qq.com)
 * @brief WIFI驱动
 * @version 0.1
 * @date 2025-10-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "bsp_wifi_driver.h"


bsp_wifi_driver_status_e bsp_wifi_driver_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());                                                          /**< 初始化底层 TCP/IP 协议栈 */
    ESP_ERROR_CHECK(esp_event_loop_create_default());                                           /**< 启动系统默认事件循环 */
    ESP_ERROR_CHECK(example_connect());                                                         /**< WiFi连接示例 */

    return BSP_WIFI_DRIVER_OK;
}





