/**
 * @file app_mqtt.h
 * @author ZB (2536566200@qq.com)
 * @brief MQTT应用
 * @version 0.1
 * @date 2025-10-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __APP_MQTT_H__
#define __APP_MQTT_H__

#include <stdio.h>
#include <string.h>
#include <sdkconfig.h>

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

typedef enum {
    APP_MQTT_STATUS_OK = 0,
    APP_MQTT_STATUS_ERROR,
}app_mqtt_status_e;

/**
 * @brief MQTT应用初始化
 * 
 * @return app_mqtt_status_e 
 */
app_mqtt_status_e app_mqtt_init(void);



#endif




