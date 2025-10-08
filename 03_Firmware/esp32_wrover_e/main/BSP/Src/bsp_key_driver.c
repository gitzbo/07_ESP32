/**
 * @file bsp_key_driver.c
 * @author ZB (2536566200@qq.com)
 * @brief 按键驱动
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "bsp_key_driver.h"

bsp_key_driver_status_e bsp_key_driver_init(void *cb)
{
    if(cb == NULL) {
        return BSP_KEY_DRIVER_ERROR;
    }

    gpio_config_t io_conf = {
        .intr_type       = GPIO_INTR_NEGEDGE,
        .mode            = GPIO_MODE_INPUT,
        .pin_bit_mask    = (1ULL << BSP_KEY_USR_PIN),
        .pull_down_en    = GPIO_PULLDOWN_DISABLE,
        .pull_up_en      = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BSP_KEY_USR_PIN, cb, (void *)BSP_KEY_USR_PIN);

    return BSP_KEY_DRIVER_OK;
}

uint8_t bsp_key_driver_get_status(uint8_t id)
{
    uint8_t ret = 0;

    if(BSP_KEY_DRIVER_ID_USR == id) {
        ret = gpio_get_level(BSP_KEY_USR_PIN);
    }

    return ret;
}





