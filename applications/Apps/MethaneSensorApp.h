/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-28     Administrator       the first version
 */
#ifndef APPLICATIONS_APPS_METHANESENSORAPP_H_
#define APPLICATIONS_APPS_METHANESENSORAPP_H_

#include "heads.h"

extern float Methane;           /* methane concentration */

extern rt_uint16_t g_methane_ppm;   /* methane ppm for cloud report */
extern rt_uint8_t  g_methane_lel;   /* methane LEL% for cloud report */
extern rt_uint8_t  g_methane_status;/* GM-402B status byte ([13], 0xAA=normal) */
extern rt_bool_t   g_methane_data_valid; /* at least one valid frame received */

rt_err_t uart2_receive_and_print(rt_int32_t timeout);

#endif /* APPLICATIONS_APPS_METHANESENSORAPP_H_ */
