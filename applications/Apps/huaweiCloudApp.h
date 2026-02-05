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


/* WiFi config */
#define HW_WIFI_SSID           "CMCC-Vm3m"
#define HW_WIFI_PASSWORD       "w3wegscf"

/* MQTT server config */
#define HW_MQTT_HOST           "c8496a111b.st1.iotda-device.cn-east-3.myhuaweicloud.com"
#define HW_MQTT_PORT           "1883"

/* Device auth - from Huawei Cloud console */
#define HW_MQTT_DEVICE_ID      "6982e30f18855b39c5f690ba_line_sensor_01"
#define HW_MQTT_CLIENT_ID      "6982e30f18855b39c5f690ba_line_sensor_01_0_0_2026020406"
#define HW_MQTT_USERNAME       "6982e30f18855b39c5f690ba_line_sensor_01"
#define HW_MQTT_PASSWORD       "369ca2bb61e28e3f491fc28d6e35f7a473186964c6ee78ef931b5b42688411f2"

/* Report topic */
#define HW_MQTT_TOPIC_REPORT   "$oc/devices/6982e30f18855b39c5f690ba_line_sensor_01/sys/properties/report"

/* Service ID - must match the service ID defined in Huawei Cloud product model */
#define SERVICE_ID_ADC      "STM32H743"

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
