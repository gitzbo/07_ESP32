/**
 * @file bsp_nvs.c
 * @author ZB (2536566200@qq.com)
 * @brief NVS驱动
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "bsp_nvs_driver.h"

bsp_nvs_driver_status_e bsp_nvs_driver_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || 
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if(ret == ESP_OK) {
        return BSP_NVS_DRIVER_OK;
    }

    return BSP_NVS_DRIVER_ERROR;
}















