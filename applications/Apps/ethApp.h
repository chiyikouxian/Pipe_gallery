/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-06-18     ideapad15s   Ethernet TCP client for fiber optic link
 */
#ifndef APPLICATIONS_APPS_ETHAPP_H_
#define APPLICATIONS_APPS_ETHAPP_H_

#include <rtthread.h>

/* TCP server config */
#define ETH_SERVER_IP           "192.168.3.10"
#define ETH_SERVER_PORT         19008

/* Device identification */
#define ETH_DEVICE_ID           "pipe_gallery_node_01"

/* Report interval (ms) */
#define ETH_REPORT_INTERVAL     3000

/* Reconnect delay (ms) */
#define ETH_RECONNECT_DELAY     5000

/**
 * @brief Initialize ethernet TCP client and start report thread
 * @return RT_EOK on success
 */
rt_err_t eth_app_init(void);

#endif /* APPLICATIONS_APPS_ETHAPP_H_ */
