/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-18     Administrator       the first version
 */

#include "freeModbusApp.h"

/* global variables */
float Voltage[3] = {0};     /* three-phase voltage */
float Current[3] = {0};     /* three-phase current */
float Flow = 0;             /* water flow */

/**
 * @brief Convert two 16-bit registers to an IEEE 754 float
 *
 * @param reg1 high 16-bit register value
 * @param reg2 low 16-bit register value
 * @return float converted float value
 *
 * Conversion steps:
 * 1. treat reg2 as unsigned 16-bit value
 * 2. combine reg1 as high 16 bits and reg2 as low 16 bits into one 32-bit value
 * 3. reinterpret the 32-bit bit pattern as IEEE 754 float
 */
static float registers_to_float(uint16_t reg1, uint16_t reg2)
{
    uint32_t combined;
    float result;

    /* step 1: make reg2 unsigned */
    uint16_t reg2_unsigned = reg2 & 0xFFFF;

    /* step 2: combine into one 32-bit value, high word first */
    combined = ((uint32_t)reg1 << 16) | reg2_unsigned;

    /* step 3: reinterpret as IEEE 754 float */
    memcpy(&result, &combined, sizeof(float));

    return result;
}

/* Ammeter voltage query: 49 03 01 00 00 06 CB BC */
/* Ammeter current query: 49 03 01 0E 00 06 AA 7F */
/* Water meter flow query: 01 03 00 01 00 02 95 CB */
/* (water meter signal line: A and B) */

void send_thread_entry(void *parameter)
{
    eMBMasterReqErrCode error_code = MB_MRE_NO_ERR;
    uint16_t reg_high, reg_low;

    while (1)
    {
        /* ==================== Read water meter flow ==================== */
        error_code = eMBMasterReqReadHoldingRegister(WATERMETER_SLAVE_ADDR,
                                                    FLOW_REG_START,
                                                    FLOW_REG_NUM,
                                                    RT_WAITING_FOREVER);

        if (error_code == MB_MRE_NO_ERR)
        {
            /* read registers from buffer: slave address 1 corresponds to buffer index 0 */
            reg_high = usMRegHoldBuf[WATERMETER_SLAVE_ADDR - 1][FLOW_REG_START];
            reg_low  = usMRegHoldBuf[WATERMETER_SLAVE_ADDR - 1][FLOW_REG_START + 1];

            /* convert to float */
            Flow = registers_to_float(reg_high, reg_low);

            /* update node data */
            node[0].Flow = Flow;

            int flow_int = (int)Flow;
            int flow_dec = (int)((Flow - flow_int) * 1000);
            if (flow_dec < 0) flow_dec = -flow_dec;
            rt_kprintf("[Water] Flow: %d.%03d m3/h (Reg[%d]=0x%04X, Reg[%d]=0x%04X)\n",
                       flow_int, flow_dec, FLOW_REG_START, reg_high, FLOW_REG_START+1, reg_low);
        }
        else
        {
            rt_kprintf("[Water] Error: %d\n", error_code);
        }

        rt_thread_mdelay(500);

        /* ==================== Read ammeter voltage ==================== */
        error_code = eMBMasterReqReadHoldingRegister(AMMETER_SLAVE_ADDR,
                                                    VOLTAGE_REG_START,
                                                    VOLTAGE_REG_NUM,
                                                    RT_WAITING_FOREVER);

        if (error_code == MB_MRE_NO_ERR)
        {
            /* slave address 73 requires MB_MASTER_TOTAL_SLAVE_NUM >= 73 */
            for (int i = 0; i < 3; i++)
            {
                reg_high = usMRegHoldBuf[AMMETER_SLAVE_ADDR - 1][VOLTAGE_REG_START + i * 2];
                reg_low  = usMRegHoldBuf[AMMETER_SLAVE_ADDR - 1][VOLTAGE_REG_START + i * 2 + 1];

                Voltage[i] = registers_to_float(reg_high, reg_low);

                int volt_int = (int)Voltage[i];
                int volt_dec = (int)((Voltage[i] - volt_int) * 100);
                if (volt_dec < 0) volt_dec = -volt_dec;
                rt_kprintf("[Ammeter] Voltage[%d]: %d.%02d V (Reg[0x%03X]=0x%04X, Reg[0x%03X]=0x%04X)\n",
                           i, volt_int, volt_dec,
                           VOLTAGE_REG_START + i*2, reg_high,
                           VOLTAGE_REG_START + i*2 + 1, reg_low);
            }
        }
        else
        {
            rt_kprintf("[Ammeter Voltage] Error: %d\n", error_code);
        }

        rt_thread_mdelay(500);

        /* ==================== Read ammeter current ==================== */
        error_code = eMBMasterReqReadHoldingRegister(AMMETER_SLAVE_ADDR,
                                                    CURRENT_REG_START,
                                                    CURRENT_REG_NUM,
                                                    RT_WAITING_FOREVER);

        if (error_code == MB_MRE_NO_ERR)
        {
            for (int i = 0; i < 3; i++)
            {
                reg_high = usMRegHoldBuf[AMMETER_SLAVE_ADDR - 1][CURRENT_REG_START + i * 2];
                reg_low  = usMRegHoldBuf[AMMETER_SLAVE_ADDR - 1][CURRENT_REG_START + i * 2 + 1];

                Current[i] = registers_to_float(reg_high, reg_low);

                int curr_int = (int)Current[i];
                int curr_dec = (int)((Current[i] - curr_int) * 1000);
                if (curr_dec < 0) curr_dec = -curr_dec;
                rt_kprintf("[Ammeter] Current[%d]: %d.%03d A (Reg[0x%03X]=0x%04X, Reg[0x%03X]=0x%04X)\n",
                           i, curr_int, curr_dec,
                           CURRENT_REG_START + i*2, reg_high,
                           CURRENT_REG_START + i*2 + 1, reg_low);
            }

            /* update node current field */
            memcpy(node[0].CH1_A, Current, sizeof(Current));
        }
        else
        {
            rt_kprintf("[Ammeter Current] Error: %d\n", error_code);
        }

        rt_kprintf("\n");  /* print separator line */
        rt_thread_mdelay(500);
    }
}

void mb_master_poll(void *parameter)
{
    eMBMasterInit(MB_RTU, PORT_NUM, PORT_BAUDRATE, PORT_PARITY);
    eMBMasterEnable();

    while (1)
    {
        eMBMasterPoll();
        rt_thread_mdelay(MB_POLL_CYCLE_MS);
    }
}
