/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-03     Sherman      the first version
 */

#include <stdint.h>
#include <stdlib.h>

#include "sensor_renesas_zmod4410.h"
#include "zmod4410_config_iaq2.h"
#include "zmod4xxx.h"
#include "zmod4xxx_hal.h"
#include "iaq_2nd_gen.h"

#define DBG_TAG "sensor.zmod4410"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define SENSOR_ETOH_RANGE_MAX (100)
#define SENSOR_ETOH_RANGE_MIN (0)

#define SENSOR_TVOC_RANGE_MAX (100)
#define SENSOR_TVOC_RANGE_MIN (0)

#define SENSOR_ECO2_RANGE_MAX (1000)
#define SENSOR_ECO2_RANGE_MIN (0)

#define SENSOR_IAQ_RANGE_MAX (100)
#define SENSOR_IAQ_RANGE_MIN (0)

struct zmod4410_device
{
    struct rt_i2c_bus_device *i2c;
    rt_uint8_t addr;
    zmod4xxx_dev_t dev;
    rt_uint8_t zmod4xxx_status;
    rt_uint8_t prod_data[ZMOD4410_PROD_DATA_LEN];
    iaq_2nd_gen_handle_t algo_handle;
};
static struct zmod4410_device zmod4410_dev;

static rt_err_t _zmod4410_init(struct rt_sensor_intf *intf)
{
    rt_int8_t ret;
    zmod4410_dev.i2c = rt_i2c_bus_device_find(intf->dev_name);
    if (zmod4410_dev.i2c == RT_NULL)
    {
        return -RT_ERROR;
    }
    ret = init_hardware(&zmod4410_dev.dev);
    if (ret)
    {
        LOG_E("Error %d during initialize hardware, exiting program!\n", ret);
        return -RT_ERROR;
    }

    zmod4410_dev.dev.i2c_addr = ZMOD4410_I2C_ADDR;
    zmod4410_dev.dev.pid = ZMOD4410_PID;
    zmod4410_dev.dev.init_conf = &zmod_sensor_type[INIT];
    zmod4410_dev.dev.meas_conf = &zmod_sensor_type[MEASUREMENT];
    zmod4410_dev.dev.prod_data = zmod4410_dev.prod_data;

    ret = zmod4xxx_read_sensor_info(&zmod4410_dev.dev);
    if (ret)
    {
        LOG_E("Error %d during reading sensor information, exiting program!\n",ret);
        goto exit;
    }

    /* Preperation of sensor */
    ret = zmod4xxx_prepare_sensor(&zmod4410_dev.dev);
    if (ret)
    {
        LOG_E("Error %d during preparation of the sensor, exiting program!\n",ret);
        goto exit;
    }

    /* One time initialization of the algorithm */
    ret = init_iaq_2nd_gen(&zmod4410_dev.algo_handle);
    if (ret)
    {
        LOG_E("Error %d when initializing algorithm, exiting program!\n", ret);
        goto exit;
    }

    return RT_EOK;
exit:
    return -RT_ERROR;
}

static void zmod4410_polling_get_data(rt_sensor_t sensor, struct rt_sensor_data *data)
{
    RT_ASSERT(sensor != RT_NULL);
    RT_ASSERT(data != RT_NULL);
    rt_uint8_t ret;
    /* Sensor target variables */
    rt_uint8_t adc_result[32] = { 0 };
    iaq_2nd_gen_results_t algo_results;
    /* Counter to check POR Event */
    rt_uint32_t polling_counter = 0;

    ret = zmod4xxx_start_measurement(&zmod4410_dev.dev);
    if (ret)
    {
        LOG_E("Error %d when starting measurement, exiting program!", ret);
        goto exit;
    }

    /* Instead of polling STATUS REGISTER, the INTERRUPT PIN can be used.
        * For more information, please look at Interrupt Usage chapter
        * in Programming Manual */
    do
    {
        ret = zmod4xxx_read_status(&zmod4410_dev.dev, &zmod4410_dev.zmod4xxx_status);
        if (ret)
        {
            LOG_E(
                "Error %d during read of sensor status, exiting program!",
                ret);
            goto exit;
        }
        /* Increase polling counter */
        polling_counter++;
        /* Delay for 200 milliseconds */
        zmod4410_dev.dev.delay_ms(200);
    }
    while ((zmod4410_dev.zmod4xxx_status & STATUS_SEQUENCER_RUNNING_MASK) &&
            (polling_counter <= ZMOD4410_IAQ2_COUNTER_LIMIT));

    /* Check if timeout has occured */
    if (ZMOD4410_IAQ2_COUNTER_LIMIT <= polling_counter)
    {
        ret = zmod4xxx_check_error_event(&zmod4410_dev.dev);
        if (ret)
        {
            LOG_E("Error %d during read of sensor status, exiting program!", ret);
            goto exit;
        }
        LOG_E("Error %d, exiting program!\n", ret);
        goto exit;
    }
    else
    {
        polling_counter = 0;
    }

    ret = zmod4xxx_read_adc_result(&zmod4410_dev.dev, adc_result);
    if (ret)
    {
        LOG_E("Error %d during read of ADC results, exiting program!\n", ret);
        goto exit;
    }

    /* calculate the algorithm */
    ret = calc_iaq_2nd_gen(&zmod4410_dev.algo_handle, &zmod4410_dev.dev, adc_result, &algo_results);
    if ((ret != IAQ_2ND_GEN_OK) && (ret != IAQ_2ND_GEN_STABILIZATION))
    {
        LOG_E("Error %d when calculating algorithm, exiting program!\n", ret);
        goto exit;
        /* IAQ 2nd Gen algorithm skips first 60 samples for stabilization */
    }
    else
    {
        switch (sensor->info.type)
        {
        case RT_SENSOR_CLASS_ETOH:
            data->data.etoh = algo_results.etoh * 1000;
            break;

        case RT_SENSOR_CLASS_TVOC:
            data->data.tvoc = algo_results.tvoc * 1000;
            break;

        case RT_SENSOR_CLASS_ECO2:
            data->data.eco2 = algo_results.eco2;
            break;

        case RT_SENSOR_CLASS_IAQ:
            data->data.iaq = algo_results.iaq * 10;
            break;

        default:
            break;
        }
        data->timestamp = rt_sensor_get_ts();

        if (ret == IAQ_2ND_GEN_STABILIZATION)
        {
            LOG_I("Warmup!");
        }
        else
        {
            LOG_I("Valid!");
        }
    }
    /* wait 1.99 seconds before starting the next measurement */
    zmod4410_dev.dev.delay_ms(1990);

exit:
    return;
}

static rt_size_t zmod4410_fetch_data(struct rt_sensor_device *sensor,
                                     void *buf,
                                     rt_size_t len)
{
    RT_ASSERT(buf);

    if (sensor->config.mode == RT_SENSOR_MODE_POLLING)
    {
        zmod4410_polling_get_data(sensor, buf);
        return 1;
    }
    else
        return 0;
}

static rt_err_t zmod4410_control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    rt_err_t result = RT_EOK;

    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    zmod4410_fetch_data,
    zmod4410_control
};

int rt_hw_zmod4410_init(const char *name, struct rt_sensor_config *cfg)
{
    rt_int8_t result;
    rt_sensor_t sensor_etoh = RT_NULL,
                sensor_tvoc = RT_NULL,
                sensor_eco2 = RT_NULL,
                sensor_iaq = RT_NULL;

    /* zmod4410 EtOH sensor register */
    sensor_etoh = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_etoh == RT_NULL)
        return -1;

    sensor_etoh->info.type       = RT_SENSOR_CLASS_ETOH;
    sensor_etoh->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor_etoh->info.model      = "zmod4410";
    sensor_etoh->info.unit       = RT_SENSOR_UNIT_PPM;
    sensor_etoh->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_etoh->info.range_max  = SENSOR_ETOH_RANGE_MAX;
    sensor_etoh->info.range_min  = SENSOR_ETOH_RANGE_MIN;
    sensor_etoh->info.period_min = 0;
    sensor_etoh->ops = &sensor_ops;
    rt_memcpy(&sensor_etoh->config, cfg, sizeof(struct rt_sensor_config));

    result = rt_hw_sensor_register(sensor_etoh,
                                   name,
                                   RT_DEVICE_FLAG_RDONLY,
                                   RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    /* zmod4410 TVOC sensor register */
    sensor_tvoc = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_tvoc == RT_NULL)
        return -1;

    sensor_tvoc->info.type       = RT_SENSOR_CLASS_TVOC;
    sensor_tvoc->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor_tvoc->info.model      = "zmod4410";
    sensor_tvoc->info.unit       = RT_SENSOR_UNIT_MGM3;
    sensor_tvoc->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_tvoc->info.range_max  = SENSOR_TVOC_RANGE_MAX;
    sensor_tvoc->info.range_min  = SENSOR_TVOC_RANGE_MIN;
    sensor_tvoc->info.period_min = 0;
    sensor_tvoc->ops = &sensor_ops;
    rt_memcpy(&sensor_tvoc->config, cfg, sizeof(struct rt_sensor_config));

    result = rt_hw_sensor_register(sensor_tvoc,
                                   name,
                                   RT_DEVICE_FLAG_RDONLY,
                                   RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    /* zmod4410 ECO2 sensor register */
    sensor_eco2 = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_eco2 == RT_NULL)
        return -1;

    sensor_eco2->info.type       = RT_SENSOR_CLASS_ECO2;
    sensor_eco2->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor_eco2->info.model      = "zmod4410";
    sensor_eco2->info.unit       = RT_SENSOR_UNIT_PPM;
    sensor_eco2->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_eco2->info.range_max  = SENSOR_ECO2_RANGE_MAX;
    sensor_eco2->info.range_min  = SENSOR_ECO2_RANGE_MIN;
    sensor_eco2->info.period_min = 0;
    sensor_eco2->ops = &sensor_ops;
    rt_memcpy(&sensor_eco2->config, cfg, sizeof(struct rt_sensor_config));

    result = rt_hw_sensor_register(sensor_eco2,
                                   name,
                                   RT_DEVICE_FLAG_RDONLY,
                                   RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    /* zmod4410 IAQ sensor register */
    sensor_iaq = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_iaq == RT_NULL)
        return -1;

    sensor_iaq->info.type       = RT_SENSOR_CLASS_IAQ;
    sensor_iaq->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor_iaq->info.model      = "zmod4410";
    sensor_iaq->info.unit       = RT_SENSOR_UNIT_ONE;
    sensor_iaq->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_iaq->info.range_max  = SENSOR_IAQ_RANGE_MAX;
    sensor_iaq->info.range_min  = SENSOR_IAQ_RANGE_MIN;
    sensor_iaq->info.period_min = 0;
    sensor_iaq->ops = &sensor_ops;
    rt_memcpy(&sensor_iaq->config, cfg, sizeof(struct rt_sensor_config));

    result = rt_hw_sensor_register(sensor_iaq,
                                   name,
                                   RT_DEVICE_FLAG_RDONLY,
                                   RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    if (RT_EOK != _zmod4410_init(&cfg->intf))
        goto __exit;

    return RT_EOK;

__exit:
    if (sensor_etoh)
        rt_free(sensor_etoh);
    if (sensor_tvoc)
        rt_free(sensor_tvoc);
    if (sensor_eco2)
        rt_free(sensor_eco2);
    if (sensor_iaq)
        rt_free(sensor_iaq);

    return -RT_ERROR;
}
