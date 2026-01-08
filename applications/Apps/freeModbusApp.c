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

// 全局变量定义
float Voltage[3] = {0};     // 三相电压
float Current[3] = {0};     // 三相电流
float Flow = 0;             // 流量

/**
 * @brief 将两个16位寄存器转换为IEEE 754单精度浮点数
 *
 * @param reg1 高16位寄存器值
 * @param reg2 低16位寄存器值
 * @return float 转换后的浮点数
 *
 * 转换步骤：
 * 1. 将reg2作16位无符号化处理
 * 2. 将reg1左移16位与reg2低位合并，得到32位整数
 * 3. 将该整数按IEEE 754格式解释为浮点数
 */
static float registers_to_float(uint16_t reg1, uint16_t reg2)
{
    uint32_t combined;
    float result;

    // 步骤1：reg2无符号化处理
    uint16_t reg2_unsigned = reg2 & 0xFFFF;

    // 步骤2：合并成32位整数（大端序：高位在前）
    combined = ((uint32_t)reg1 << 16) | reg2_unsigned;

    // 步骤3：将32位整数按位模式直接解释为IEEE 754浮点数
    memcpy(&result, &combined, sizeof(float));

    return result;
}


/* 电表请求报文 电压:49 03 01 00 00 06 CB BC */
/* 电表请求报文 电流:49 03 01 0E 00 06 AA 7F */
/* 水表请求报文 流量:01 03 00 01 00 02 95 CB */

void send_thread_entry(void *parameter)
{
    eMBMasterReqErrCode error_code = MB_MRE_NO_ERR;
    uint16_t reg_high, reg_low;

    while (1)
    {
        /* ==================== 读取水表流量 ==================== */
        error_code = eMBMasterReqReadHoldingRegister(WATERMETER_SLAVE_ADDR,
                                                    FLOW_REG_START,
                                                    FLOW_REG_NUM,
                                                    RT_WAITING_FOREVER);

        if (error_code == MB_MRE_NO_ERR)
        {
            // 从缓冲区读取寄存器值（注意：从站地址1对应数组索引0）
            reg_high = usMRegHoldBuf[WATERMETER_SLAVE_ADDR - 1][FLOW_REG_START];
            reg_low  = usMRegHoldBuf[WATERMETER_SLAVE_ADDR - 1][FLOW_REG_START + 1];

            // 转换为浮点数
            Flow = registers_to_float(reg_high, reg_low);

            // 更新节点数据
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

        /* ==================== 读取电表电压 ==================== */
        error_code = eMBMasterReqReadHoldingRegister(AMMETER_SLAVE_ADDR,
                                                    VOLTAGE_REG_START,
                                                    VOLTAGE_REG_NUM,
                                                    RT_WAITING_FOREVER);

        if (error_code == MB_MRE_NO_ERR)
        {
            // 注意：从站地址73需要确保缓冲区数组范围足够
            // MB_MASTER_TOTAL_SLAVE_NUM 默认是16，需要修改或使用额外处理
            // 此处假设已经修改为MB_MASTER_TOTAL_SLAVE_NUM >= 73

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

        /* ==================== 读取电表电流 ==================== */
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

            // 更新节点数据（将电流值拷贝到node结构体的CH1_A字段）
            memcpy(node[0].CH1_A, Current, sizeof(Current));
        }
        else
        {
            rt_kprintf("[Ammeter Current] Error: %d\n", error_code);
        }

        rt_kprintf("\n");  // 打印空行分隔
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
