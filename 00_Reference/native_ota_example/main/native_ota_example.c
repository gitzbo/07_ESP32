/* OTA example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "protocol_examples_common.h"
#include "errno.h"

#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

#define BUFFSIZE 1024
#define HASH_LEN 32 /* SHA-256 digest length */

static const char *TAG = "native_ota_example";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

#define OTA_URL_SIZE 256

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);          /**< 关闭 HTTP 连接，但仍保留所有 HTTP 请求资源 */
    esp_http_client_cleanup(client);        /**< 逆初始化init 并清空资源 */
}

static void __attribute__((noreturn)) task_fatal_error(void)
{
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}

static void print_sha256 (const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s: %s", label, hash_print);
}

static void infinite_loop(void)
{
    int i = 0;
    ESP_LOGI(TAG, "When a new firmware is available on the server, press the reset button to download it");
    while(1) {
        ESP_LOGI(TAG, "Waiting for a new firmware ... %d", ++i);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void ota_example_task(void *pvParameter)
{
    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(TAG, "Starting OTA example task");

    const esp_partition_t *configured = esp_ota_get_boot_partition();       /**< 获取boot分区信息 */
    const esp_partition_t *running = esp_ota_get_running_partition();       /**< 获取运行分区信息 */

    /**< 判断偏移是否一致 */
    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08"PRIx32", but running from offset 0x%08"PRIx32,
                 configured->address,                                       /**< 配置文件偏移地址 */
                 running->address);                                         /**< 运行文件偏移地址 */
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08"PRIx32")",
             running->type,                                                 /**< 运行分区类型 */
             running->subtype,                                              /**< 运行分区子类型 */
             running->address);                                             /**< 运行分区地址 */

    esp_http_client_config_t config = {
        .url                = CONFIG_EXAMPLE_FIRMWARE_UPG_URL,              /**< 固件地址 */
        .cert_pem           = (char *)server_cert_pem_start,                /**< SSL 服务器证书，PEM 格式字符串，如果客户端需要验证服务器 */
        .timeout_ms         = CONFIG_EXAMPLE_OTA_RECV_TIMEOUT,              /**< 网络超时时间 */
        .keep_alive_enable  = true,                                         /**< 保持活动超时 */
    };

    /**< 从标准输入配置示例固件升级URL */
#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL_FROM_STDIN
    char url_buf[OTA_URL_SIZE];
    if (strcmp(config.url, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout();
        fgets(url_buf, OTA_URL_SIZE, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        config.url = url_buf;
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong firmware upgrade image url");
        abort();
    }
#endif

    /**< 配置示例_跳过通用名称检查 */
#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    esp_http_client_handle_t client = esp_http_client_init(&config);        /**< 初始化HTTP */
    if (client == NULL) {   
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");  
        task_fatal_error();                                                 /**< 线程错误 */
    }   
    err = esp_http_client_open(client, 0);                                  /**< 此函数将打开连接，写入所有头部字符串并返回 */
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", 
                        esp_err_to_name(err));                              /**< 返回错误代码并以字符串格式显示 */
        esp_http_client_cleanup(client);                                    /**< 逆初始化init 并清空资源 */
        task_fatal_error();                                                 /**< 消除线程 */
    }
    esp_http_client_fetch_headers(client);

    update_partition = esp_ota_get_next_update_partition(NULL);             /**< 获取下一个需要升级的分区 */
    assert(update_partition != NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%"PRIx32,
             update_partition->subtype,                                     /**< 更新数据的分区类型 */
             update_partition->address);                                    /**< 更新数据的地址 */

    int binary_file_length = 0;
    /*deal with all receive packet*/
    bool image_header_was_checked = false;
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);     /**< 从 HTTP 流中读取数据 */
        if (data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            http_cleanup(client);                                                   /**< 清空并退出http */
            task_fatal_error();                                                     /**< 线程错误销毁此线程 */
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // check current version with downloading
                    memcpy(&new_app_info,                                                                       /**< 新分区信息 */
                            &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)],   /**< 新分区文件的原始信息 */
                            sizeof(esp_app_desc_t));
                    ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);                            /**< 新分区的固件版本 */

                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {              /**< 获取当前运行的分区信息 */
                        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);                /**< 打印当前运行的分区信息 */
                    }

                    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();             /**< 返回最后无效的分区 */
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {     /**< 获取当前最后的分区信息 */
                        ESP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);           /**< 打印当前最后的分区信息 */
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        /**< 判断版本是否与上个版本相同，相同不进行升级 */
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                            ESP_LOGW(TAG, "New version is the same as invalid version.");
                            ESP_LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                            ESP_LOGW(TAG, "The firmware has been rolled back to the previous version.");
                            http_cleanup(client);           /**< 关闭并清空http连接 */
                            infinite_loop();                /**< 等待新固件 */
                        }
                    }
#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
                    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                        ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
                        http_cleanup(client);               /**< 关闭并清空http连接 */
                        infinite_loop();                    /**< 等待新固件 */
                    }
#endif

                    image_header_was_checked = true;        /**< 数据包为真 */

                    err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);  /**< 开始对指定分区进行ota升级 */
                    if (err != ESP_OK) {
                        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                        http_cleanup(client);                                                           /**< 关闭并清空http连接 */
                        esp_ota_abort(update_handle);                                                   /**< 终止ota升级并释放内存 */
                        task_fatal_error();                                                             /**< 删除线程 */
                    }
                    ESP_LOGI(TAG, "esp_ota_begin succeeded");
                } else {
                    ESP_LOGE(TAG, "received package is not fit len");
                    http_cleanup(client);                                                               /**< 关闭并清空HTTP连接 */
                    esp_ota_abort(update_handle);                                                       /**< 终止OTA升级并释放内存 */
                    task_fatal_error();                                                                 /**< 删除线程 */
                }
            }
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);               /**< OTA对指定分区进行写入 */
            if (err != ESP_OK) {
                http_cleanup(client);                                                                   /**< 关闭并清空http连接 */
                esp_ota_abort(update_handle);                                                           /**< 终止OTA升级并释放内存 */
                task_fatal_error();                                                                     /**< 删除线程 */
            }
            binary_file_length += data_read;
            ESP_LOGD(TAG, "Written image length %d", binary_file_length);                               /**< 打印写入大小 */
        } else if (data_read == 0) {
           /*
            * As esp_http_client_read never returns negative error code, we rely on
            * `errno` to check for underlying transport connectivity closure if any
            */
            if (errno == ECONNRESET || errno == ENOTCONN) {                                             /**< 检测errno判断是否已经断开连接 */
                ESP_LOGE(TAG, "Connection closed, errno = %d", errno);
                break;
            }
            if (esp_http_client_is_complete_data_received(client) == true) {                            /**< 检查响应中的所有数据是否已被无错误地读取 */
                ESP_LOGI(TAG, "Connection closed");
                break;
            }
        }
    }
    ESP_LOGI(TAG, "Total Write binary data length: %d", binary_file_length);                            /**< 向目标写入的数据长度 */
    if (esp_http_client_is_complete_data_received(client) != true) {                                    /**< 检查响应中的所有数据是否已被无错误地读取 */
        ESP_LOGE(TAG, "Error in receiving complete file");
        http_cleanup(client);                                                                           /**< 关闭并清空http连接 */
        esp_ota_abort(update_handle);                                                                   /**< 终止OTA升级并释放内存 */
        task_fatal_error();                                                                             /**< 删除线程 */
    }

    err = esp_ota_end(update_handle);                                                                   /**< 完成并验证OTA */
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        } else {
            ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        http_cleanup(client);                                                                           /**< 关闭并清空http连接 */
        task_fatal_error();                                                                             /**< 终止OTA升级并释放内存 */
    }

    err = esp_ota_set_boot_partition(update_partition);                                                 /**< 为新的启动分区配置OTA数据 */
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);                                                                           /**< 关闭并清空http连接 */
        task_fatal_error();                                                                             /**< 删除线程 */
    }
    ESP_LOGI(TAG, "Prepare to restart system!");
    esp_restart();                                                                                      /**< 芯片重启 */
    return ;
}

static bool diagnostic(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type    = GPIO_INTR_DISABLE;                                                           /**< 关闭GPIO中断 */
    io_conf.mode         = GPIO_MODE_INPUT;                                                             /**< GPIO输入模式 */
    io_conf.pin_bit_mask = (1ULL << CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);                                    /**< GPIO掩码 */
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;                                                       /**< 关闭下拉 */
    io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;                                                          /**< 启动上拉 */
    gpio_config(&io_conf);                                                                              /**< 配置该GPIO */

    
    for(uint8_t i = 0; i < 5; i++) {
        ESP_LOGI(TAG, "Diagnostics (%d sec)...", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);                                                          /**< 等待5秒 */
    }
    

    bool diagnostic_is_ok = gpio_get_level(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);                             /**< 获取按键状态 */

    gpio_reset_pin(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);                                                     /**< 重置GPIO参数 */
    return diagnostic_is_ok;
}

void app_main(void)
{
    ESP_LOGI(TAG, "OTA example app_main start");

    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;

    // get sha256 digest for the partition table
    partition.address   = ESP_PARTITION_TABLE_OFFSET;                   /**< 获取分区表偏移 */
    partition.size      = ESP_PARTITION_TABLE_MAX_LEN;                  /**< 获取分区表长度 */
    partition.type      = ESP_PARTITION_TYPE_DATA;                      /**< 获取数据位置 */
    esp_partition_get_sha256(&partition, sha_256);                      /**< 获取分区表sha256 */
    print_sha256(sha_256, "SHA-256 for the partition table: ");

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;                        /**< 获取bl偏移  */
    partition.size      = ESP_PARTITION_TABLE_OFFSET;                   /**< 获取分区表偏移 */
    partition.type      = ESP_PARTITION_TYPE_APP;                       /**< 获取应用位置 */
    esp_partition_get_sha256(&partition, sha_256);                      /**< 获取sha256 */
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256); /**< 获取运行地址的sha256 */
    print_sha256(sha_256, "SHA-256 for current firmware: ");

    const esp_partition_t *running = esp_ota_get_running_partition();   /**< 获取运行地址 */
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            // run diagnostic function ...
            bool diagnostic_is_ok = diagnostic();                       /**< 检测按键是否进行 */
            if (diagnostic_is_ok) {
                ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
                esp_ota_mark_app_valid_cancel_rollback();               /**< 表明运行良好不进行回滚 */
            } else {
                ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
                esp_ota_mark_app_invalid_rollback_and_reboot();         /**< 需要进行回滚版本 */
            }
        }
    }

    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    ESP_ERROR_CHECK(esp_netif_init());                                              /**< 初始化TCP/IP参数 */
    ESP_ERROR_CHECK(esp_event_loop_create_default());                               /**< 创建默认事件循环 */

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());                                             /**< 启动WiFi连接示例 */

#if CONFIG_EXAMPLE_CONNECT_WIFI
    /* Ensure to disable any WiFi power save mode, this allows best throughput
     * and hence timings for overall OTA operation.
     */
    esp_wifi_set_ps(WIFI_PS_NONE);                                                  /**< 禁用WiFi节能模式 */
#endif // CONFIG_EXAMPLE_CONNECT_WIFI

    xTaskCreate(&ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);        /**< 创建ota线程 */
}
