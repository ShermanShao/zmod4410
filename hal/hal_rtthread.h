/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-15     Sherman      first version
 */

#ifndef _HAL_RTTHREAD_H
#define _HAL_RTTHREAD_H

#include <rtthread.h>
#include "zmod4xxx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialize the target hardware
 * @param   [in] dev pointer to the device
 * @return  error code
 * @retval  0 success
 * @retval  "!= 0" error
 */
int8_t init_hardware(zmod4xxx_dev_t *dev);

/**
 * @brief   Check if any key is pressed
 * @retval  1 pressed
 * @retval  0 not pressed
 */
int8_t is_key_pressed(void);

/**
 * @brief   deinitialize target hardware
 * @return  error code
 * @retval  0 success
 * @retval  "!= 0" error
 */
int8_t deinit_hardware(void);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_RTTHREAD_H */
