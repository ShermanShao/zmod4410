/*******************************************************************************
 * Copyright (c) 2021 Renesas Electronics Corporation
 * All Rights Reserved.
 *
 * This code is proprietary to Renesas, and is license pursuant to the terms and
 * conditions that may be accessed at:
 * https://www.renesas.com/eu/en/document/msc/renesas-software-license-terms-gas-sensor-software
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   This is an example for the ZMOD4410 gas sensor using the IAQ 2nd Gen library.
 * @version 2.2.0
 * @author Renesas Electronics Corporation
 **/

#include "zmod4410_config_iaq2.h"
#include "zmod4xxx.h"
#include "zmod4xxx_cleaning.h"
#include "zmod4xxx_hal.h"
#include "iaq_2nd_gen.h"

static int demo()
{
    int8_t ret;
    zmod4xxx_dev_t dev;
    /* Counter to check POR Event */
    uint32_t polling_counter = 0;

    /* Sensor target variables */
    uint8_t zmod4xxx_status;
    uint8_t prod_data[ZMOD4410_PROD_DATA_LEN];
    uint8_t adc_result[32] = { 0 };
    iaq_2nd_gen_handle_t algo_handle;
    iaq_2nd_gen_results_t algo_results;

    /****TARGET SPECIFIC FUNCTION ****/
    /*
     * Customers have to write their own init_hardware function in
     * init_hardware (located in dependencies/zmod4xxx_api/HAL directory)
     * variable of dev has *read, *write and *delay function pointer. Three
     * functions have to be generated for corresponding hardware and assign
     * in init_hardware. For more information, read the programming manual.
     */
    ret = init_hardware(&dev);
    if (ret) {
        printf("Error %d during initialize hardware, exiting program!\n", ret);
        goto exit;
    }
    /****TARGET SPECIFIC FUNCTION ****/

    /* Sensor Related Data */
    dev.i2c_addr = ZMOD4410_I2C_ADDR;
    dev.pid = ZMOD4410_PID;
    dev.init_conf = &zmod_sensor_type[INIT];
    dev.meas_conf = &zmod_sensor_type[MEASUREMENT];
    dev.prod_data = prod_data;

    ret = zmod4xxx_read_sensor_info(&dev);
    if (ret) {
        printf("Error %d during reading sensor information, exiting program!\n",
               ret);
        goto exit;
    }

    /* This function starts the cleaning procedure. It's
    * recommended to be executed after product assembly. This
    * helps to clean the metal oxide surface from assembly.
    * IMPORTANT NOTE: The cleaning procedure can be run only once
    * during the modules lifetime and takes 10 minutes. */

    //ret = zmod4xxx_cleaning_run(&dev);
    //if (ret) {
    //    printf("Error %d during cleaning procedure, exiting program!\n", ret);
    //    goto exit;
    //}

    /* Preperation of sensor */
    ret = zmod4xxx_prepare_sensor(&dev);
    if (ret) {
        printf("Error %d during preparation of the sensor, exiting program!\n",
               ret);
        goto exit;
    }

    /* One time initialization of the algorithm */
    ret = init_iaq_2nd_gen(&algo_handle);
    if (ret) {
        printf("Error %d when initializing algorithm, exiting program!\n", ret);
        goto exit;
    }

    ret = zmod4xxx_start_measurement(&dev);
    if (ret) {
        printf("Error %d when starting measurement, exiting program!\n", ret);
        goto exit;
    }

    printf("Evaluate measurements in a loop. Press any key to quit.\n\n");
    do {
        /* Instead of polling STATUS REGISTER, the INTERRUPT PIN can be used.
         * For more information, please look at Interrupt Usage chapter
         * in Programming Manual */
        do {
            ret = zmod4xxx_read_status(&dev, &zmod4xxx_status);
            if (ret) {
                printf(
                    "Error %d during read of sensor status, exiting program!\n",
                    ret);
                goto exit;
            }
            /* Increase polling counter */
            polling_counter++;
            /* Delay for 200 milliseconds */
            dev.delay_ms(200);
        } while ((zmod4xxx_status & STATUS_SEQUENCER_RUNNING_MASK) &&
                 (polling_counter <= ZMOD4410_IAQ2_COUNTER_LIMIT));

        /* Check if timeout has occured */
        if (ZMOD4410_IAQ2_COUNTER_LIMIT <= polling_counter) {
            ret = zmod4xxx_check_error_event(&dev);
            if (ret) {
                printf(
                    "Error %d during read of sensor status, exiting program!\n",
                    ret);
                goto exit;
            }
            printf("Error %d, exiting program!\n", ret);
            goto exit;
        } else {
            polling_counter = 0;
        }

        ret = zmod4xxx_read_adc_result(&dev, adc_result);
        if (ret) {
            printf("Error %d during read of ADC results, exiting program!\n",
                   ret);
            goto exit;
        }

        /* calculate the algorithm */
        ret = calc_iaq_2nd_gen(&algo_handle, &dev, adc_result, &algo_results);
        if ((ret != IAQ_2ND_GEN_OK) && (ret != IAQ_2ND_GEN_STABILIZATION)) {
            printf("Error %d when calculating algorithm, exiting program!\n",
                   ret);
            goto exit;
            /* IAQ 2nd Gen algorithm skips first 60 samples for stabilization */
        } else {
            printf("*********** Measurements ***********\n");
            for (int i = 0; i < 13; i++) {
                printf(" Rmox[%d] = ", i);
                printf("%.3f kOhm\n", algo_results.rmox[i] / 1e3);
            }
            printf(" log_Rcda = %5.3f logOhm\n", algo_results.log_rcda);
            printf(" EtOH = %6.3f ppm\n", algo_results.etoh);
            printf(" TVOC = %6.3f mg/m^3\n", algo_results.tvoc);
            printf(" eCO2 = %4.0f ppm\n", algo_results.eco2);
            printf(" IAQ  = %4.1f\n", algo_results.iaq);
            if (ret == IAQ_2ND_GEN_STABILIZATION) {
                printf("Warmup!\n");
            } else {
                printf("Valid!\n");
            }
            printf("************************************\n");
        }

        /* wait 1.99 seconds before starting the next measurement */
        dev.delay_ms(1990);

        /* start a new measurement before result calculation */
        ret = zmod4xxx_start_measurement(&dev);
        if (ret) {
            printf("Error %d when starting measurement, exiting program!\n",
                   ret);
            goto exit;
        }
        dev.delay_ms(50);

    } while (!is_key_pressed());

exit:
    ret = deinit_hardware();
    if (ret) {
        printf("Error %d during deinitialize hardware, exiting program\n", ret);
        return ret;
    }
    return 0;
}

#include <rtthread.h>
void zmod_entry(void *param)
{
    int ret = demo();
    if(ret)
    {
        printf("\r\nzmod demo return %d \r\n",ret);
    }
}

void zmod_demo()
{
    rt_thread_t tid = rt_thread_create("zmod",zmod_entry, RT_NULL, 2048, 10, 50);
    if(tid)
    {
        rt_thread_startup(tid);
    }
}

MSH_CMD_EXPORT(zmod_demo,zmod4410 demo);
