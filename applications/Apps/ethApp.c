/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-06-18     ideapad15s   Ethernet TCP client for fiber optic link
 */
#include "ethApp.h"
#include "huaweiCloudApp.h"
#include "sht30App.h"
#include "o2SensorApp.h"
#include "linesensor.h"
#include <rtthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#define DBG_TAG "EthApp"
#define DBG_LVL DBG_ERROR
#include <rtdbg.h>

static char eth_json_buf[512];
static int  eth_sock = -1;

static int append_float(char *buf, int size, const char *key, float val)
{
    int i = (int)val;
    int d = (int)((val - i) * 100);
    if (d < 0) d = -d;
    return rt_snprintf(buf, size, "\"%s\":%d.%02d", key, i, d);
}

static int eth_tcp_connect(void)
{
    struct sockaddr_in server_addr;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LOG_E("socket create failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ETH_SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(ETH_SERVER_IP);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) < 0)
    {
        LOG_E("connect to %s:%d failed", ETH_SERVER_IP, ETH_SERVER_PORT);
        closesocket(sock);
        return -1;
    }

    LOG_I("connected to %s:%d", ETH_SERVER_IP, ETH_SERVER_PORT);
    return sock;
}

static int build_sensor_json(void)
{
    int pos;

    pos = rt_snprintf(eth_json_buf, sizeof(eth_json_buf),
        "{\"device_id\":\"%s\",\"data\":{"
        "\"ch0\":%u,\"ch1\":%u,\"ch3\":%u,\"ch4\":%u,\"ch5\":%u,"
        "\"methane_ppm\":%u,\"methane_lel\":%u,",
        ETH_DEVICE_ID,
        (unsigned int)g_adc_ch0,
        (unsigned int)g_adc_ch1,
        (unsigned int)g_adc_ch3,
        (unsigned int)g_adc_ch4,
        (unsigned int)g_adc_ch5,
        (unsigned int)g_methane_ppm,
        (unsigned int)g_methane_lel);

    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "voltage_a", Voltage[0]);
    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, ",");
    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "voltage_b", Voltage[1]);
    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, ",");
    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "voltage_c", Voltage[2]);
    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, ",");
    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "current_a", Current[0]);
    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, ",");
    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "current_b", Current[1]);
    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, ",");
    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "current_c", Current[2]);
    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, ",");
    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "flow", Flow);
    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, ",");
    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "temperature", g_temperature_c);
    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, ",");
    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "humidity", g_humidity_rh);
    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, ",");
    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "o2", g_o2_concentration);
    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, ",");
    pos += append_float(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos, "displacement", Displacement);

    pos += rt_snprintf(eth_json_buf + pos,
        sizeof(eth_json_buf) - pos,
        ",\"flame\":%d}}\n", (int)Flame);

    return pos;
}

static void eth_client_thread_entry(void *parameter)
{
    int len, ret;
    int fail_count = 0;

    LOG_I("Ethernet TCP client thread started.");
    rt_thread_mdelay(8000);

    while (1)
    {
        if (eth_sock < 0)
        {
            eth_sock = eth_tcp_connect();
            if (eth_sock < 0)
            {
                rt_thread_mdelay(ETH_RECONNECT_DELAY);
                continue;
            }
            fail_count = 0;
        }

        len = build_sensor_json();
        ret = send(eth_sock, eth_json_buf, len, 0);
        if (ret <= 0)
        {
            fail_count++;
            LOG_W("TCP send failed (%d/3)", fail_count);
            if (fail_count >= 3)
            {
                LOG_E("Too many failures, reconnecting...");
                closesocket(eth_sock);
                eth_sock = -1;
            }
        }
        else
        {
            fail_count = 0;
            LOG_D("Sent %d bytes via ETH", ret);
        }

        rt_thread_mdelay(ETH_REPORT_INTERVAL);
    }
}

rt_err_t eth_app_init(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("eth_tcp",
                           eth_client_thread_entry,
                           RT_NULL,
                           2048,
                           24,
                           10);
    if (tid == RT_NULL)
    {
        LOG_E("Failed to create ETH TCP thread!");
        return -RT_ERROR;
    }

    rt_thread_startup(tid);
    LOG_I("ETH TCP client thread created.");
    return RT_EOK;
}
