/**
 * @file bsp_key_driver.h
 * @author ZB (2536566200@qq.com)
 * @brief 按键驱动
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __BSP_KEY_DRIVER_H__
#define __BSP_KEY_DRIVER_H__

#include <stdint.h>
#include "driver/gpio.h"

#define BSP_KEY_CHOOSE_FLAG 1       /**< 使用实体按键还是触摸按键 */

#if BSP_KEY_CHOOSE_FLAG
#define BSP_KEY_USR_PIN     GPIO_NUM_27      /**< 用户按键引脚 */
#else
#define BSP_KEY_TOUCH_PIN   GPIO_NUM_27      /**< 触摸按键引脚 */
#endif

/**< 运行状态 */
typedef enum {
    BSP_KEY_DRIVER_OK = 0,
    BSP_KEY_DRIVER_ERROR,
}bsp_key_driver_status_e;

/**< 具体按键响应 */
typedef enum {
    BSP_KEY_DRIVER_ID_NULL = 0,
    BSP_KEY_DRIVER_ID_USR,
    BSP_KEY_DRIVER_ID_TOUCH,
}bsp_key_driver_id_e;

/**
 * @brief 按键驱动初始化
 * 
 * @param cb 中断回调函数
 * @return bsp_key_driver_status_e 运行状态
 */
bsp_key_driver_status_e bsp_key_driver_init(void *cb);

/**
 * @brief 获取按键状态
 * 
 * @param id 按键id
 * @return uint8_t 该按键状态 
 */
uint8_t bsp_key_driver_get_status(uint8_t id);




#endif




