/**
 * @file app_mqtt.c
 * @author ZB (2536566200@qq.com)
 * @brief MQTT应用
 * @version 0.1
 * @date 2025-10-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "app_mqtt.h"

static const char *TAG = "app_mqtt";
static void _mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

app_mqtt_status_e app_mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {   
        .broker.address.uri = "mqtt://bemfa.com:9501",                      /**< URL网址 */
        // .credentials.username = "8k7mi8MPi002",                          /**< 设备名称 */
        .credentials.client_id = "",                                        /**< 私钥 */
        // .credentials.authentication.password = NULL,                     /**< token密码 */
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);                          /**< 配置MQTT句柄 */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, _mqtt_event_handler, NULL);        /**< 注册MQTT回调 */
    esp_mqtt_client_start(client);                                                              /**< 启动MQTT客户端 */

    return APP_MQTT_STATUS_OK;
}

/*
 * @brief 事件处理程序已注册，以接收 MQTT 事件
 *
 * @param handler_args 用户数据已登记到该活动
 * @param base 处理程序的事件基础（在此示例中始终为 MQTT 基础）
 * @param event_id 接收到的事件的ID
 * @param event_data 事件的数据
 */
static void _mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:      /**< 连接事件 */
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        msg_id = esp_mqtt_client_subscribe(client, "8k7mi8MPi002/up", 0);                           /**< 订阅主题消息质量 */
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "8k7mi8MPi002/set", 0);                          /**< 订阅主题消息质量 */
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        /**< 客户端向代理发送发布消息 */
        msg_id = esp_mqtt_client_publish(client,                                                    /**< MQTT对象 */
                                        "8k7mi8MPi002/up",                                          /**< TOPIC主题 */
                                        "{\"msg\":\"test\"}",                                       /**< 数据 */
                                        0,                                                          /**< 长度 0表示载荷长度 */
                                        0,                                                          /**< QOS服务质量等级 */
                                        0);                                                         /**< 保留标志 */
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");                             /**< 退订主题 */
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:   /**< 断开连接事件 */
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:     /**< 订阅事件 */
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "8k7mi8MPi002/up", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:   /**< 取消订阅事件 */
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:      /**< 已发布事件 */
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:           /**< 数据事件 */
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:          /**< 错误事件 */
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            // log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            // log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            // log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            // ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}




