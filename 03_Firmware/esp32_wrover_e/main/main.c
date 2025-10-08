/**
 * @file main.c
 * @author ZB (2536566200@qq.com)
 * @brief main文件
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

/**< static */
static void _board_init(void);
static void _app_init(void);

/**< 中断 */
extern void IRAM_ATTR key_isr_handler(void *arg);

void app_main(void)
{
    _board_init();                              /**< 外设初始化 */
    _app_init();                                /**< 应用初始化 */
    
    while(1) {
        vTaskDelay(1000);
    }
}

static void _board_init(void)
{
    bsp_nvs_driver_init();                      /**< nvs初始化 */
    bsp_key_driver_init(key_isr_handler);       /**< 按键初始化 */
    bsp_ble_driver_init();                      /**< BLE初始化 */
}

static void _app_init(void)
{
    userShellInit();                            /**< Shell应用初始化 */
    app_key_init();                             /**< 按键应用初始化 */
    app_ble_init();                             /**< 蓝牙应用初始化 */
}



