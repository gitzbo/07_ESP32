/**
 * @file app_ota.c
 * @author ZB (2536566200@qq.com)
 * @brief OTA应用
 * @version 0.1
 * @date 2025-10-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "app_ota.h"

static const char *TAG = "app_ota";                                             /**< OTA标志 */
static char ota_write_data[BUFFSIZE + 1] = { 0 };                               /**< 准备写入闪存的 OTA 数据写入缓冲区 */
static void _app_ota_task(void *pvParameter);

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");  /**< 服务开始 */
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");      /**< 服务结束 */

app_ota_status_e app_ota_init(void)
{
    /**< 1.判断程序是否回滚 */
    // uint8_t sha_256[HASH_LEN] = { 0 };
    // esp_partition_t partition;

    // // get sha256 digest for the partition table
    // partition.address   = ESP_PARTITION_TABLE_OFFSET;                   /**< 获取分区表偏移 */
    // partition.size      = ESP_PARTITION_TABLE_MAX_LEN;                  /**< 获取分区表长度 */
    // partition.type      = ESP_PARTITION_TYPE_DATA;                      /**< 获取数据位置 */
    // esp_partition_get_sha256(&partition, sha_256);                      /**< 获取分区表sha256 */
    // print_sha256(sha_256, "SHA-256 for the partition table: ");

    // // get sha256 digest for bootloader
    // partition.address   = ESP_BOOTLOADER_OFFSET;                        /**< 获取bl偏移  */
    // partition.size      = ESP_PARTITION_TABLE_OFFSET;                   /**< 获取分区表偏移 */
    // partition.type      = ESP_PARTITION_TYPE_APP;                       /**< 获取应用位置 */
    // esp_partition_get_sha256(&partition, sha_256);                      /**< 获取sha256 */
    // print_sha256(sha_256, "SHA-256 for bootloader: ");

    // // get sha256 digest for running partition
    // esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256); /**< 获取运行地址的sha256 */
    // print_sha256(sha_256, "SHA-256 for current firmware: ");

    // const esp_partition_t *running = esp_ota_get_running_partition();   /**< 获取运行地址 */
    // esp_ota_img_states_t ota_state;
    // if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
    //     if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
    //         // run diagnostic function ...
    //         bool diagnostic_is_ok = diagnostic();                       /**< 检测按键是否进行 */
    //         if (diagnostic_is_ok) {
    //             ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
    //             esp_ota_mark_app_valid_cancel_rollback();               /**< 表明运行良好不进行回滚 */
    //         } else {
    //             ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
    //             esp_ota_mark_app_invalid_rollback_and_reboot();         /**< 需要进行回滚版本 */
    //         }
    //     }
    // }

    /**< 2.创建OTA线程 */
    xTaskCreate(_app_ota_task,                  /**< 按键线程 */
                APP_OTA_TASK_NAME,              /**< 线程名称 */
                APP_OTA_TASK_STACK,             /**< 线程堆栈大小 */
                APP_OTA_TASK_PARAM,             /**< 用户参数 */
                APP_OTA_TASK_PRIO,              /**< 线程优先级 */
                APP_OTA_TASK_HANDLER);          /**< 线程句柄 */    

    return APP_OTA_STATUS_OK;
}

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

static void _app_ota_task(void *pvParameter)
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

    esp_http_client_handle_t client = esp_http_client_init(&config);                                            /**< 初始化HTTP */
    if (client == NULL) {                                       
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");                                      
        task_fatal_error();                                                                                     /**< 线程错误 */
    }                                       
    err = esp_http_client_open(client, 0);                                                                      /**< 此函数将打开连接，写入所有头部字符串并返回 */
    if (err != ESP_OK) {                                    
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s",                                     
                        esp_err_to_name(err));                                                                  /**< 返回错误代码并以字符串格式显示 */
        esp_http_client_cleanup(client);                                                                        /**< 逆初始化init 并清空资源 */
        task_fatal_error();                                                                                     /**< 消除线程 */
    }                                   
    esp_http_client_fetch_headers(client);                                  
                                    
    update_partition = esp_ota_get_next_update_partition(NULL);                                                 /**< 获取下一个需要升级的分区 */
    assert(update_partition != NULL);                                   
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%"PRIx32,                                    
             update_partition->subtype,                                                                         /**< 更新数据的分区类型 */
             update_partition->address);                                                                        /**< 更新数据的地址 */

    int binary_file_length = 0;
    /*deal with all receive packet*/
    bool image_header_was_checked = false;
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);                                 /**< 从 HTTP 流中读取数据 */
        if (data_read < 0) {                            
            ESP_LOGE(TAG, "Error: SSL data read error");                            
            http_cleanup(client);                                                                               /**< 清空并退出http */
            task_fatal_error();                                                                                 /**< 线程错误销毁此线程 */
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









