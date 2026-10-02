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
#ifndef APPLICATIONS_APPS_STRESSSENSORAPP_H_
#define APPLICATIONS_APPS_STRESSSENSORAPP_H_

#include "heads.h"

/********************* RDF-TC25 / RDD-DH MODBUS ************************/
/*
 * Bus parameters: Modbus RTU, 9600 baud, 8 data bits, no parity, 1 stop bit.
 * The instrument ships/currently responds as slave 1. Slave 1 is already
 * used by the water meter on this bus, so configure the RDD-DH as slave 3
 * before connecting both devices to UART3 at the same time.
 */
#define STRESS_SLAVE_ADDR           3

/* Holding registers read with function code 0x03. */
#define STRESS_STATUS_REG           0x0008
#define STRESS_VALUE_REG            0x0050
#define STRESS_VALUE_REG_NUM        2
#define STRESS_UNIT_REG             0x0068

#define STRESS_DECIMAL_MASK         0x0007

#define STRESS_UNIT_NONE            0
#define STRESS_UNIT_G               1
#define STRESS_UNIT_KG              2
#define STRESS_UNIT_T               3
#define STRESS_UNIT_N               4

/* Latest measurement in newtons and protocol metadata. */
extern float g_stress_value_n;
extern int32_t g_stress_raw_value;
extern uint16_t g_stress_status;
extern uint16_t g_stress_unit;
extern uint8_t g_stress_decimal_places;
extern rt_bool_t g_stress_sensor_online;
extern rt_bool_t g_stress_value_valid;

/* Called by the shared UART3 Modbus polling thread. */
rt_err_t stress_sensor_poll(void);

#endif /* APPLICATIONS_APPS_STRESSSENSORAPP_H_ */
