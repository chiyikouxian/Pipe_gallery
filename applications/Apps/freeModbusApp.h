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
#define PORT_NUM        3       /* use UART3 for Modbus communication */
#define PORT_BAUDRATE   9600
#define PORT_PARITY     MB_PAR_NONE

#define MB_POLL_THREAD_PRIORITY  10
#define MB_SEND_THREAD_PRIORITY  RT_THREAD_PRIORITY_MAX - 1

#define MB_SEND_REG_START  1
#define MB_SEND_REG_NUM    10
#define MB_READ_REG_START  0
#define MB_READ_NUM        10

#define MODBUS_NUM   1

/*************************** Ammeter MODBUS ****************************/
#define AMMETER_SLAVE_ADDR      73          /* ammeter slave address */
#define VOLTAGE_REG_START       0x100       /* voltage start register: 256 */
#define VOLTAGE_REG_NUM         6           /* 3 voltage values, each occupies 2 registers */
#define CURRENT_REG_START       0x10E       /* current start register: 270 */
#define CURRENT_REG_NUM         6           /* 3 current values, each occupies 2 registers */

extern float Voltage[3];        /* three-phase voltage: A, B, C */
extern float Current[3];        /* three-phase current: A, B, C */
/*************************** Ammeter MODBUS ****************************/

/*************************** Water meter MODBUS ************************/
#define WATERMETER_SLAVE_ADDR   1           /* water meter slave address */
#define FLOW_REG_START          1           /* instant flow */
#define FLOW_REG_NUM            2           /* one float value occupies 2 registers */

extern float Flow;              /* water flow */
/*************************** Water meter MODBUS ************************/

#define MB_POLL_CYCLE_MS   500

void send_thread_entry(void *parameter);
void mb_master_poll(void *parameter);

#endif /* APPLICATIONS_APPS_FREEMODBUSAPP_H_ */
