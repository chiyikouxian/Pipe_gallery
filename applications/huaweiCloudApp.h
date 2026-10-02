/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-02-04     ideapad15s   Huawei Cloud IoT application
 */
#ifndef APPLICATIONS_APPS_HUAWEICLOUDAPP_H_
#define APPLICATIONS_APPS_HUAWEICLOUDAPP_H_

#include <rtthread.h>

/*============================================================================
 * Huawei Cloud IoTDA connection config - modify according to your device
 *============================================================================*/
/*ESP8266 CONNECT
 * EN---3.3V
 * ESP8266TX---PA11
 * ESP8266RX---PA12
 */


/* WiFi config - UPDATE WITH YOUR WiFi CREDENTIALS */
#define HW_WIFI_SSID           "dmail"
#define HW_WIFI_PASSWORD       "edqn8272"

/* MQTT server config */
#define HW_MQTT_HOST           "2f49ff4f87.st1.iotda-device.cn-east-3.myhuaweicloud.com"
#define HW_MQTT_PORT           "1883"    /* Use 1883 for non-encrypted MQTT (ESP8266 compatible) */

/* Device auth - from Huawei Cloud console */
#define HW_MQTT_DEVICE_ID      "6abf5210e094d61592745069_1776006881"
#define HW_MQTT_CLIENT_ID      "6abf5210e094d61592745069_1776006881_0_0_2026100206"
#define HW_MQTT_USERNAME       "6abf5210e094d61592745069_1776006881"
#define HW_MQTT_PASSWORD       "4c5f30d1439aac277d128a2902f20df4df38636004840eb4a1e3e6a792061916"

/* Report topic */
#define HW_MQTT_TOPIC_REPORT   "$oc/devices/6abf5210e094d61592745069_1776006881/sys/properties/report"

/* Service ID - must match the service ID defined in Huawei Cloud product model */
#define SERVICE_ID_SENSOR   "Smarthome"    /* All sensor data service ID */

/* Report interval (ms) */
#define CLOUD_REPORT_INTERVAL   3000    /* report every 3 seconds */

/*============================================================================
 * Global ADC data - for cloud module to read
 *============================================================================*/
extern rt_uint32_t g_adc_ch0;   /* ADC channel 0 value */
extern rt_uint32_t g_adc_ch1;   /* ADC channel 1 value */
extern rt_uint32_t g_adc_ch3;   /* ADC channel 3 value */
extern rt_uint32_t g_adc_ch4;   /* ADC channel 4 value */
extern rt_uint32_t g_adc_ch5;   /* ADC channel 5 value */

/* Methane sensor data - updated by MethaneSensorApp.c */
extern rt_uint16_t g_methane_ppm;   /* Methane concentration (ppm) */
extern rt_uint8_t  g_methane_lel;   /* Lower explosive limit (LEL%) */

/* Modbus sensor data - updated by freeModbusApp.c */
extern float Voltage[3];   /* 3-phase voltage (V) */
extern float Current[3];   /* 3-phase current (A) */
extern float Flow;          /* Water flow (m3/h) */

/*============================================================================
 * Function declarations
 *============================================================================*/

/**
 * @brief Huawei Cloud thread entry function
 * @param parameter Thread parameter (unused)
 */
void huawei_cloud_thread_entry(void *parameter);

/**
 * @brief Initialize Huawei Cloud upload function
 * @return RT_EOK success, other values failure
 */
rt_err_t huawei_cloud_init(void);

#endif /* APPLICATIONS_APPS_HUAWEICLOUDAPP_H_ */
