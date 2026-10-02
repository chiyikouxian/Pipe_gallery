/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-04-13     ideapad15s   ATK-LORA-01 module via UART5
 */
#ifndef APPLICATIONS_APPS_LORAAPP_H_
#define APPLICATIONS_APPS_LORAAPP_H_

#include <rtthread.h>

#define LORA_SEND_INTERVAL      5000    /* send interval (ms) */

void lora_thread_entry(void *parameter);

#endif /* APPLICATIONS_APPS_LORAAPP_H_ */
