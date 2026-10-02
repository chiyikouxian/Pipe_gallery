/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2026-09-05     Administrator   initial stress sensor application
 * 2026-09-20     Codex           use RDF-TC25/RDD-DH register protocol
 */

#include "stressSensorApp.h"
#include "freeModbusApp.h"

float g_stress_value_n = 0.0f;
int32_t g_stress_raw_value = 0;
uint16_t g_stress_status = 0;
uint16_t g_stress_unit = STRESS_UNIT_NONE;
uint8_t g_stress_decimal_places = 0;
rt_bool_t g_stress_sensor_online = RT_FALSE;
rt_bool_t g_stress_value_valid = RT_FALSE;

static const uint32_t stress_pow10_table[] =
{
    1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U
};

static eMBMasterReqErrCode stress_read_registers(uint8_t slave_addr,
                                                  uint16_t *status,
                                                  uint16_t *unit,
                                                  uint16_t *reg_high,
                                                  uint16_t *reg_low)
{
    eMBMasterReqErrCode error_code;

    error_code = eMBMasterReqReadHoldingRegister(slave_addr,
                                                 STRESS_STATUS_REG,
                                                 1,
                                                 RT_WAITING_FOREVER);
    if (error_code != MB_MRE_NO_ERR)
        return error_code;
    *status = usMRegHoldBuf[slave_addr - 1][STRESS_STATUS_REG];

    error_code = eMBMasterReqReadHoldingRegister(slave_addr,
                                                 STRESS_UNIT_REG,
                                                 1,
                                                 RT_WAITING_FOREVER);
    if (error_code != MB_MRE_NO_ERR)
        return error_code;
    *unit = usMRegHoldBuf[slave_addr - 1][STRESS_UNIT_REG];

    error_code = eMBMasterReqReadHoldingRegister(slave_addr,
                                                 STRESS_VALUE_REG,
                                                 STRESS_VALUE_REG_NUM,
                                                 RT_WAITING_FOREVER);
    if (error_code != MB_MRE_NO_ERR)
        return error_code;

    *reg_high = usMRegHoldBuf[slave_addr - 1][STRESS_VALUE_REG];
    *reg_low = usMRegHoldBuf[slave_addr - 1][STRESS_VALUE_REG + 1];
    return MB_MRE_NO_ERR;
}

static rt_bool_t stress_convert_to_n(int32_t raw_value,
                                     uint8_t decimal_places,
                                     uint16_t unit,
                                     float *value_n)
{
    float displayed_value;

    if (decimal_places >= (sizeof(stress_pow10_table) /
                           sizeof(stress_pow10_table[0])))
        return RT_FALSE;

    displayed_value = (float)raw_value /
                      (float)stress_pow10_table[decimal_places];

    switch (unit)
    {
    case STRESS_UNIT_N:
        *value_n = displayed_value;
        break;
    case STRESS_UNIT_T:
        *value_n = displayed_value * 9806.65f;
        break;
    case STRESS_UNIT_KG:
        *value_n = displayed_value * 9.80665f;
        break;
    case STRESS_UNIT_G:
        *value_n = displayed_value * 0.00980665f;
        break;
    default:
        return RT_FALSE;
    }

    return RT_TRUE;
}

static rt_bool_t stress_decode_measurement(uint16_t status,
                                           uint16_t unit,
                                           uint16_t reg_high,
                                           uint16_t reg_low,
                                           int32_t *raw_value,
                                           uint8_t *decimal_places,
                                           float *value_n)
{
    uint32_t combined = ((uint32_t)reg_high << 16) | (uint32_t)reg_low;

    *raw_value = (int32_t)combined;
    *decimal_places = (uint8_t)(status & STRESS_DECIMAL_MASK);
    return stress_convert_to_n(*raw_value, *decimal_places, unit, value_n);
}

rt_err_t stress_sensor_poll(void)
{
    eMBMasterReqErrCode error_code;
    uint16_t status;
    uint16_t unit;
    uint16_t reg_high;
    uint16_t reg_low;
    int32_t raw_value;
    uint8_t decimal_places;
    float value_n;

    error_code = stress_read_registers(STRESS_SLAVE_ADDR,
                                       &status,
                                       &unit,
                                       &reg_high,
                                       &reg_low);
    if (error_code != MB_MRE_NO_ERR)
    {
        g_stress_sensor_online = RT_FALSE;
        g_stress_value_valid = RT_FALSE;
        return -RT_ERROR;
    }

    g_stress_sensor_online = RT_TRUE;
    g_stress_status = status;
    g_stress_unit = unit;
    g_stress_value_valid = stress_decode_measurement(status,
                                                     unit,
                                                     reg_high,
                                                     reg_low,
                                                     &raw_value,
                                                     &decimal_places,
                                                     &value_n);
    g_stress_raw_value = raw_value;
    g_stress_decimal_places = decimal_places;

    if (g_stress_value_valid)
        g_stress_value_n = value_n;

    return RT_EOK;
}
