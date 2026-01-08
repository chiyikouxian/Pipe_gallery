/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-16     Administrator       the first version
 */
#ifndef APPLICATIONS_APPS_FREEMODBUSAPP_H_
#define APPLICATIONS_APPS_FREEMODBUSAPP_H_

#include "heads.h"

extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];
extern USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];


#define SLAVE_ADDR      1
#define PORT_NUM        3       // 使用UART3进行Modbus通信
#define PORT_BAUDRATE   9600

#define PORT_PARITY     MB_PAR_NONE

#define MB_POLL_THREAD_PRIORITY  10
#define MB_SEND_THREAD_PRIORITY  RT_THREAD_PRIORITY_MAX - 1

#define MB_SEND_REG_START  1
#define MB_SEND_REG_NUM    10
#define MB_READ_REG_START  0
#define MB_READ_NUM    10

#define MODBUS_NUM   1

/*************************** 电表MODBUS ****************************/
#define AMMETER_SLAVE_ADDR      73          // 电表从站地址
#define VOLTAGE_REG_START       0x100       // 电压起始寄存器（256）
#define VOLTAGE_REG_NUM         6           // 3个电压值，每个占2个寄存器
#define CURRENT_REG_START       0x10E       // 电流起始寄存器（270）
#define CURRENT_REG_NUM         6           // 3个电流值，每个占2个寄存器

extern float Voltage[3];        // 三相电压：A相、B相、C相
extern float Current[3];        // 三相电流：A相、B相、C相
/*************************** 电表MODBUS *****************************/

/*************************** 水表MODBUS ****************************/
#define WATERMETER_SLAVE_ADDR      1        // 水表从站起始地址
#define FLOW_REG_START  1                   // 瞬时流量
#define FLOW_REG_NUM    2                   // 1个流量值，每个占两个寄存器

extern float Flow;             // 流量
/*************************** 水表MODBUS *****************************/

#define MB_POLL_CYCLE_MS   500

void send_thread_entry(void *parameter);
void mb_master_poll(void *parameter);

#endif /* APPLICATIONS_APPS_FREEMODBUSAPP_H_ */
