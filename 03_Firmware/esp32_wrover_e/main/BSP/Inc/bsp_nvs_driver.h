/**
 * @file bsp_nvs.h
 * @author ZB (2536566200@qq.com)
 * @brief NVS驱动
 * @version 0.1
 * @date 2025-10-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __BSP_NVS_DRIVER_H__
#define __BSP_NVS_DRIVER_H__

#include "nvs_flash.h"

/**< NVS运行状态 */
typedef enum {
    BSP_NVS_DRIVER_OK = 0,
    BSP_NVS_DRIVER_ERROR
}bsp_nvs_driver_status_e;

/**
 * @brief nvs驱动初始化
 * 
 * @return bsp_nvs_driver_status_e 运行状态
 */
bsp_nvs_driver_status_e bsp_nvs_driver_init(void);








#endif
