/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-02-04     ideapad15s   Huawei Cloud IoT - ADC line sensor data upload
 */
#include "huaweiCloudApp.h"
#include "uartApp.h"
#include <string.h>
#include <stdio.h>

#define DBG_TAG "HuaweiCloud"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/*============================================================================
 * Global ADC data - updated by linesensor.c
 *============================================================================*/
rt_uint32_t g_adc_ch0 = 0;
rt_uint32_t g_adc_ch1 = 0;
rt_uint32_t g_adc_ch3 = 0;
rt_uint32_t g_adc_ch4 = 0;
rt_uint32_t g_adc_ch5 = 0;

/*============================================================================
 * Private variables
 *============================================================================*/
static rt_uint8_t cloud_task_step = 0;      /* current task step */
static char at_cmd_buf[600];                /* AT command buffer */
static char json_buf[256];                  /* JSON data buffer */

/*============================================================================
 * Private functions
 *============================================================================*/

/**
 * @brief Send AT command and wait for response
 * @param cmd AT command
 * @param expect Expected response string
 * @param timeout Timeout in ms
 * @return 1 success, 0 failure
 */
static int send_at_cmd(const char *cmd, const char *expect, rt_uint32_t timeout)
{
    rt_uint32_t start_tick;

    /* Clear receive buffer */
    uart4_buffer_clear();

    /* Send AT command */
    uart4_send(cmd);
    uart4_send("\r\n");

    /* Wait for response */
    start_tick = rt_tick_get();
    while ((rt_tick_get() - start_tick) < rt_tick_from_millisecond(timeout))
    {
        rt_thread_mdelay(100);
        if (strstr(uart4_recived_data, expect) != RT_NULL)
        {
            LOG_D("AT OK: %s", cmd);
            return 1;
        }
        if (strstr(uart4_recived_data, "ERROR") != RT_NULL)
        {
            LOG_E("AT ERROR: %s", cmd);
            return 0;
        }
    }

    LOG_E("AT TIMEOUT: %s", cmd);
    return 0;
}

/**
 * @brief Wait for expected response
 * @param expect Expected response string
 * @param timeout Timeout in ms
 * @return 1 success, 0 failure
 */
static int wait_response(const char *expect, rt_uint32_t timeout)
{
    rt_uint32_t start_tick = rt_tick_get();

    while ((rt_tick_get() - start_tick) < rt_tick_from_millisecond(timeout))
    {
        rt_thread_mdelay(100);
        if (strstr(uart4_recived_data, expect) != RT_NULL)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Build ADC data report JSON
 * @note Report 5-channel ADC line sensor data
 */
static void build_adc_json(void)
{
    /* Build Huawei Cloud property report JSON format */
    /* {"services":[{"service_id":"STM32H743","properties":{"ch0":xxx,"ch1":xxx,...}}]} */
    rt_snprintf(json_buf, sizeof(json_buf),
        "{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"ch0\":%u,\"ch1\":%u,\"ch3\":%u,\"ch4\":%u,\"ch5\":%u}}]}",
        SERVICE_ID_ADC,
        (unsigned int)g_adc_ch0,
        (unsigned int)g_adc_ch1,
        (unsigned int)g_adc_ch3,
        (unsigned int)g_adc_ch4,
        (unsigned int)g_adc_ch5);
}

/*============================================================================
 * Other sensor report functions (commented out, uncomment when needed)
 *============================================================================*/

#if 0  /* ===== Other sensor report code - currently disabled ===== */

/**
 * @brief Build Node data report JSON (Modbus collected data)
 * @note Includes: Flow, Flame, Methane, 3-phase current
 * @note Before use:
 *       1. Create corresponding service and properties on Huawei Cloud
 *       2. Modify SERVICE_ID to match
 *       3. Ensure node[] array has valid data
 */
/*
static void build_node_json(void)
{
    extern Node node[NODENUM];  // Reference node data from NodeApp.h

    // Report node[0] data
    rt_snprintf(json_buf, sizeof(json_buf),
        "{\"services\":[{\"service_id\":\"Node_1\",\"properties\":{"
        "\"Flow\":%.2f,"
        "\"Flame\":%d,"
        "\"Methane\":%.2f,"
        "\"current_A\":%.2f,"
        "\"current_B\":%.2f,"
        "\"current_C\":%.2f"
        "}}]}",
        node[0].Flow,
        (int)node[0].Flame,
        node[0].Methane,
        node[0].CH1_A[0],
        node[0].CH1_A[1],
        node[0].CH1_A[2]);
}
*/

/**
 * @brief Build methane sensor data report JSON
 * @note Before use:
 *       1. Create methane sensor service on Huawei Cloud
 *       2. Add properties: methane_ppm(int), methane_lel(int)
 */
/*
static void build_methane_json(rt_uint16_t ppm, rt_uint8_t lel)
{
    rt_snprintf(json_buf, sizeof(json_buf),
        "{\"services\":[{\"service_id\":\"MethaneSensor\",\"properties\":{"
        "\"methane_ppm\":%u,"
        "\"methane_lel\":%u"
        "}}]}",
        (unsigned int)ppm,
        (unsigned int)lel);
}
*/

/**
 * @brief Build flame sensor data report JSON
 */
/*
static void build_flame_json(rt_uint8_t flame_status)
{
    rt_snprintf(json_buf, sizeof(json_buf),
        "{\"services\":[{\"service_id\":\"FlameSensor\",\"properties\":{"
        "\"flame\":%u"
        "}}]}",
        (unsigned int)flame_status);
}
*/

/**
 * @brief Report all sensor data (combined report)
 * @note Use when reporting multiple sensor data to the same Topic
 */
/*
static void report_all_sensors(void)
{
    extern Node node[NODENUM];

    // Report multiple services at once
    rt_snprintf(json_buf, sizeof(json_buf),
        "{\"services\":["
        "{\"service_id\":\"STM32H743\",\"properties\":{\"ch0\":%u,\"ch1\":%u,\"ch3\":%u,\"ch4\":%u,\"ch5\":%u}},"
        "{\"service_id\":\"Node_1\",\"properties\":{\"Flow\":%.2f,\"Flame\":%d,\"Methane\":%.2f}}"
        "]}",
        (unsigned int)g_adc_ch0, (unsigned int)g_adc_ch1,
        (unsigned int)g_adc_ch3, (unsigned int)g_adc_ch4, (unsigned int)g_adc_ch5,
        node[0].Flow, (int)node[0].Flame, node[0].Methane);
}
*/

#endif /* ===== Other sensor report code end ===== */

/*============================================================================
 * Public functions
 *============================================================================*/

/**
 * @brief Huawei Cloud thread entry function
 *
 * Workflow:
 * Step 0: Check ESP8266
 * Step 1: Set WiFi mode
 * Step 2: Connect WiFi
 * Step 3: Clean MQTT state
 * Step 4: Configure MQTT user
 * Step 5: Configure MQTT ClientID
 * Step 6: Connect MQTT server
 * Step 7: Report ADC data in loop
 */
void huawei_cloud_thread_entry(void *parameter)
{
    LOG_I("Huawei Cloud thread started.");

    /* Wait for UART4 thread to initialize */
    rt_thread_mdelay(1000);

    while (1)
    {
        switch (cloud_task_step)
        {
        case 0:  /* Check ESP8266 */
            LOG_I("Step 0: Check ESP8266...");
            if (send_at_cmd("AT", "OK", 2000))
            {
                cloud_task_step = 1;
            }
            else
            {
                LOG_E("ESP8266 not responding, retrying...");
                rt_thread_mdelay(3000);
            }
            break;

        case 1:  /* Set WiFi Station mode */
            LOG_I("Step 1: Set WiFi mode...");
            if (send_at_cmd("AT+CWMODE=1", "OK", 2000))
            {
                cloud_task_step = 2;
            }
            else
            {
                rt_thread_mdelay(1000);
            }
            break;

        case 2:  /* Connect WiFi */
            LOG_I("Step 2: Connect to WiFi [%s]...", HW_WIFI_SSID);
            rt_snprintf(at_cmd_buf, sizeof(at_cmd_buf),
                "AT+CWJAP=\"%s\",\"%s\"", HW_WIFI_SSID, HW_WIFI_PASSWORD);
            uart4_buffer_clear();
            uart4_send(at_cmd_buf);
            uart4_send("\r\n");

            /* WiFi connection needs longer timeout */
            if (wait_response("WIFI GOT IP", 15000))
            {
                LOG_I("WiFi connected!");
                cloud_task_step = 3;
            }
            else
            {
                LOG_E("WiFi connect failed, retrying...");
                rt_thread_mdelay(3000);
            }
            break;

        case 3:  /* Clean old MQTT state before configuring */
            LOG_I("Step 3: Clean MQTT state...");
            send_at_cmd("AT+MQTTCLEAN=0", "OK", 2000);
            rt_thread_mdelay(1000);
            cloud_task_step = 4;
            break;

        case 4:  /* Configure MQTT user (must be before MQTTCLIENTID) */
            LOG_I("Step 4: Configure MQTT user...");
            rt_snprintf(at_cmd_buf, sizeof(at_cmd_buf),
                "AT+MQTTUSERCFG=0,1,\"NULL\",\"%s\",\"%s\",0,0,\"\"",
                HW_MQTT_USERNAME, HW_MQTT_PASSWORD);
            if (send_at_cmd(at_cmd_buf, "OK", 3000))
            {
                cloud_task_step = 5;
            }
            else
            {
                rt_thread_mdelay(1000);
            }
            break;

        case 5:  /* Configure MQTT ClientID */
            LOG_I("Step 5: Configure MQTT ClientID...");
            rt_snprintf(at_cmd_buf, sizeof(at_cmd_buf),
                "AT+MQTTCLIENTID=0,\"%s\"", HW_MQTT_CLIENT_ID);
            if (send_at_cmd(at_cmd_buf, "OK", 2000))
            {
                cloud_task_step = 6;
            }
            else
            {
                LOG_W("MQTTCLIENTID failed, retrying from clean...");
                cloud_task_step = 3;
                rt_thread_mdelay(2000);
            }
            break;

        case 6:  /* Connect MQTT server */
            LOG_I("Step 6: Connect to MQTT server...");
            rt_snprintf(at_cmd_buf, sizeof(at_cmd_buf),
                "AT+MQTTCONN=0,\"%s\",%s,1", HW_MQTT_HOST, HW_MQTT_PORT);
            uart4_buffer_clear();
            uart4_send(at_cmd_buf);
            uart4_send("\r\n");

            if (wait_response("MQTTCONNECTED", 10000))
            {
                LOG_I("MQTT connected! Device is online.");
                cloud_task_step = 7;
            }
            else
            {
                LOG_E("MQTT connect failed, retrying from step 3...");
                cloud_task_step = 3;
                rt_thread_mdelay(3000);
            }
            break;

        case 7:  /* Report ADC data */
            /* Build AT+MQTTPUB command with properly escaped JSON */
            /* ESP8266 AT requires: \" for quotes, \, for commas inside the data field */
            rt_snprintf(at_cmd_buf, sizeof(at_cmd_buf),
                "AT+MQTTPUB=0,\"%s\","
                "\"{\\\"services\\\":[{\\\"service_id\\\":\\\"%s\\\"\\,"
                "\\\"properties\\\":{\\\"ch0\\\":%u\\,"
                "\\\"ch1\\\":%u\\,"
                "\\\"ch3\\\":%u\\,"
                "\\\"ch4\\\":%u\\,"
                "\\\"ch5\\\":%u}}]}\",0,0",
                HW_MQTT_TOPIC_REPORT,
                SERVICE_ID_ADC,
                (unsigned int)g_adc_ch0,
                (unsigned int)g_adc_ch1,
                (unsigned int)g_adc_ch3,
                (unsigned int)g_adc_ch4,
                (unsigned int)g_adc_ch5);

            if (send_at_cmd(at_cmd_buf, "OK", 5000))
            {
                LOG_I("Data reported: ch0=%u, ch1=%u, ch3=%u, ch4=%u, ch5=%u",
                    (unsigned int)g_adc_ch0, (unsigned int)g_adc_ch1,
                    (unsigned int)g_adc_ch3, (unsigned int)g_adc_ch4,
                    (unsigned int)g_adc_ch5);
            }
            else
            {
                LOG_W("Data report failed, checking connection...");
                /* Check if disconnected */
                if (strstr(uart4_recived_data, "MQTTDISCONNECTED") != RT_NULL)
                {
                    LOG_E("MQTT disconnected! Reconnecting...");
                    cloud_task_step = 3;  /* Reconnect */
                }
            }

            /* Wait for report interval */
            rt_thread_mdelay(CLOUD_REPORT_INTERVAL);
            break;

        default:
            cloud_task_step = 0;
            break;
        }
    }
}

/**
 * @brief Initialize Huawei Cloud upload function
 * @return RT_EOK success, other values failure
 */
rt_err_t huawei_cloud_init(void)
{
    rt_thread_t cloud_thread;

    /* Create cloud upload thread */
    cloud_thread = rt_thread_create("hw_cloud",
                                     huawei_cloud_thread_entry,
                                     RT_NULL,
                                     4096,    /* stack size */
                                     26,      /* priority */
                                     10);

    if (cloud_thread != RT_NULL)
    {
        rt_thread_startup(cloud_thread);
        LOG_I("Huawei Cloud thread created.");
        return RT_EOK;
    }
    else
    {
        LOG_E("Failed to create Huawei Cloud thread!");
        return -RT_ERROR;
    }
}
