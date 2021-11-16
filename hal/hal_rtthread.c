/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-15     Sherman      first version
 */

#include "hal_rtthread.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <rtdbg.h>

#define ZMOD4410_NAME      "i2c1"
struct rt_i2c_bus_device *i2c_bus;

#define USER_INPUT	"P105"
static rt_uint8_t is_key = 0;

/**
 * @brief Sleep for some time. Depending on target and application this can \n
 *        be used to go into power down or to do task switching.
 * @param [in] ms will sleep for at least this number of milliseconds
 */
static void rtthread_sleep(uint32_t ms)
{
    rt_thread_mdelay(ms);
}

/* I2C communication */
/**
 * @brief Read a register over I2C
 * @param [in] i2c_addr 7-bit I2C slave address of the ZMOD45xx
 * @param [in] reg_addr address of internal register to read
 * @param [out] buf destination buffer; must have at least a size of len*uint8_t
 * @param [in] len number of bytes to read
 * @return error code
 */
static int8_t rtthread_i2c_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *buf, uint8_t len)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr = (rt_uint16_t)i2c_addr;
    msgs[0].buf = &reg_addr;
    msgs[0].len = 1;
    msgs[0].flags = RT_I2C_WR | RT_I2C_NO_STOP;

    msgs[1].addr = (rt_uint16_t)i2c_addr;
    msgs[1].buf = buf;
    msgs[1].len = len;
    msgs[1].flags = RT_I2C_RD;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) == 2)
    {
        return ZMOD4XXX_OK;
    }
    else
    {
        LOG_E("i2c_read ERROR_I2C");
        return ERROR_I2C;
    }
}

/**
 * @brief Write a register over I2C using protocol described in Renesas App Note \n
 *        ZMOD4xxx functional description.
 * @param [in] i2c_addr 7-bit I2C slave address of the ZMOD4xxx
 * @param [in] reg_addr address of internal register to write
 * @param [in] buf source buffer; must have at least a size of len*uint8_t
 * @param [in] len number of bytes to write
 * @return error code
 */
static int8_t rtthread_i2c_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *buf, uint8_t len)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr = (rt_uint16_t)i2c_addr;
    msgs[0].buf = &reg_addr;
    msgs[0].len = 1;
    msgs[0].flags = RT_I2C_WR | RT_I2C_NO_STOP;

    msgs[1].addr = (rt_uint16_t)i2c_addr;
    msgs[1].buf = buf;
    msgs[1].len = len;
    msgs[1].flags = RT_I2C_WR | RT_I2C_IGNORE_NACK;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) == 2)
    {
        return ZMOD4XXX_OK;
    }
    else
    {
        LOG_E("i2c_write ERROR_I2C");
        return ERROR_I2C;
    }
}

static void irq_callback(void *args)
{
    is_key = 1;
}

/**
 * @brief   Initialize the target hardware
 * @param   [in] dev pointer to the device
 * @return  error code
 * @retval  0 success
 * @retval  "!= 0" error
 */
int8_t init_hardware(zmod4xxx_dev_t *dev)
{
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(ZMOD4410_NAME);
    if(i2c_bus == RT_NULL)
    {
        rt_kprintf("can't find %s device!\n", ZMOD4410_NAME);
        return ERROR_NULL_PTR;
    }

    dev->read = rtthread_i2c_read;
    dev->write = rtthread_i2c_write;
    dev->delay_ms = rtthread_sleep;

    /* init */
    rt_uint32_t pin = rt_pin_get(USER_INPUT);
    rt_kprintf("\n pin number : 0x%04X \n", pin);
    rt_err_t err = rt_pin_attach_irq(pin, PIN_IRQ_MODE_RISING, irq_callback, RT_NULL);
    if(RT_EOK != err)
    {
        rt_kprintf("\n attach irq failed. \n");
    }
    err = rt_pin_irq_enable(pin, PIN_IRQ_ENABLE);
    if(RT_EOK != err)
    {
        rt_kprintf("\n enable irq failed. \n");
    }
    return ZMOD4XXX_OK;
}

/**
 * @brief   Check if any key is pressed
 * @retval  1 pressed
 * @retval  0 not pressed
 */
int8_t is_key_pressed(void)
{
    if(is_key)
    {
        is_key = 0;
        return 1;
    }
    return 0;
}

/**
 * @brief   deinitialize target hardware
 * @return  error code
 * @retval  0 success
 * @retval  "!= 0" error
 */
int8_t deinit_hardware(void)
{
    return ZMOD4XXX_OK;
}
