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
 * @file zmod4410_config_iaq2.h
 * @brief This is the configuration for zmod4410 module - iaq_2nd_gen library
 * @version 2.2.0
 * @author Renesas Electronics Corporation
 */

#ifndef _ZMOD4410_CONFIG_IAQ_2ND_GEN_H
#define _ZMOD4410_CONFIG_IAQ_2ND_GEN_H

#include <stdio.h>
#include <zmod4xxx_types.h>

#define INIT        0
#define MEASUREMENT 1

#define ZMOD4410_PID      0x2310
#define ZMOD4410_I2C_ADDR 0x32

#define ZMOD4410_H_ADDR 0x40
#define ZMOD4410_D_ADDR 0x50
#define ZMOD4410_M_ADDR 0x60
#define ZMOD4410_S_ADDR 0x68

#define ZMOD4410_PROD_DATA_LEN 7

/* < Define limit of counter > */
#define ZMOD4410_IAQ2_COUNTER_LIMIT 10U

uint8_t data_set_4410i[] = {
                                0x00, 0x50,
                                0x00, 0x28, 0xC3, 0xE3,
                                0x00, 0x00, 0x80, 0x40};

uint8_t data_set_4410_iaq_2nd_gen[] = {
                                       0x00, 0x50, 0xFF, 0x38,
                                       0xFE, 0xD4, 0xFE, 0x70,
                                       0xFE, 0x0C, 0xFD, 0xA8,
                                       0xFD, 0x44, 0xFC, 0xE0,
                                       0x00, 0x52, 0x02, 0x67,
                                       0x00, 0xCD, 0x03, 0x34,
                                       0x23, 0x03, 0xA3, 0x43,
                                       0x00, 0x00, 0x06, 0x49,
                                       0x06, 0x4A, 0x06, 0x4B,
                                       0x06, 0x4C, 0x06, 0x4D,
                                       0x06, 0x4E, 0x06, 0x97,
                                       0x06, 0xD7, 0x06, 0x57,
                                       0x06, 0x4E, 0x06, 0x4D,
                                       0x06, 0x4C, 0x06, 0x4B,
                                       0x06, 0x4A, 0x86, 0x59};


zmod4xxx_conf zmod_sensor_type[] = {
    [INIT] = {
        .start = 0x80,
        .h = { .addr = ZMOD4410_H_ADDR, .len = 2, .data_buf = &data_set_4410i[0]},
        .d = { .addr = ZMOD4410_D_ADDR, .len = 2, .data_buf = &data_set_4410i[2]},
        .m = { .addr = ZMOD4410_M_ADDR, .len = 2, .data_buf = &data_set_4410i[4]},
        .s = { .addr = ZMOD4410_S_ADDR, .len = 4, .data_buf = &data_set_4410i[6]},
        .r = { .addr = 0x97, .len = 4},
    },

    [MEASUREMENT] = {
        .start = 0x80,
        .h = {.addr = ZMOD4410_H_ADDR, .len = 16, .data_buf = &data_set_4410_iaq_2nd_gen[0]},
        .d = {.addr = ZMOD4410_D_ADDR, .len = 8, .data_buf = &data_set_4410_iaq_2nd_gen[16]},
        .m = {.addr = ZMOD4410_M_ADDR, .len = 4, .data_buf = &data_set_4410_iaq_2nd_gen[24]},
        .s = {.addr = ZMOD4410_S_ADDR, .len = 32, .data_buf = &data_set_4410_iaq_2nd_gen[28]},
        .r = {.addr = 0x97, .len = 32},
        .prod_data_len = ZMOD4410_PROD_DATA_LEN,
    },
};

#endif //_ZMOD4410_CONFIG_IAQ_2ND_GEN_H
