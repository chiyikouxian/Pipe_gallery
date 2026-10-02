/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-24     Administrator       the first version
 */
#ifndef APPLICATIONS_APPS_WIFIAPP_H_
#define APPLICATIONS_APPS_WIFI_H_

#include "heads.h"

/*************************************************
 * 结尾有\r\n
1. AT+CWMODE=1
2. AT+CWJAP="cdut-yb","cdutyb218"
3. AT+MQTTUSERCFG=0,1,"NULL","67bedcb724d772325520a4c7_pipe_1","43846f59b20582164b1e50874aa3ef815af9d01c9cc35a0dc74232f72f163683",0,0,""
4. AT+MQTTCLIENTID=0,"67bedcb724d772325520a4c7_pipe_1_0_0_2025022804"
5. AT+MQTTCONN=0,"48931f72a4.st1.iotda-device.cn-north-4.myhuaweicloud.com",1883,1
6. AT+MQTTSUB=0,"oc/devices/67bedcb724d772325520a4c7_pipe_1/sys/properties/report",1
7. AT+MQTTPUB=0,"$oc/devices/67bedcb724d772325520a4c7_pipe_1/sys/properties/report","{\"services\":[{\"service_id\":\"Node_1\"\,\"properties\":{\"Flow\":12.23\,\"Flame\": 1\,\"Methane\":3.23\,\"current_A\":2\,\"current_B\":3.2\,\"current_C\":3.5}}]}",0,0
*************************************************/

#define WIFI_SSID    "Win10-2018PYBHT 2565"   // Wi-Fi 名称    "cdut-yb"    "Win10-2018PYBHT 2565"
#define WIFI_PASSWORD "1&H09h78" // Wi-Fi 密码   "cdutyb218"   "1&H09h78"
#define MQTT_USERNAME "67bedcb724d772325520a4c7_pipe_1" // 三元组用户名
#define MQTT_PASSWORD "43846f59b20582164b1e50874aa3ef815af9d01c9cc35a0dc74232f72f163683"//密码
#define MQTT_CLIENT_ID "67bedcb724d772325520a4c7_pipe_1_0_0_2025022804"//ClientID
#define MQTT_IP "48931f72a4.st1.iotda-device.cn-north-4.myhuaweicloud.com" // MQTT地址
//#define MQTT_SUB_TOPIC "oc/devices/67bedcb724d772325520a4c7_pipe_1/sys/properties/report" // MQTT订阅topic

void if_is_ok();
void wifi_thread_entry(void *parameter);

#endif /* APPLICATIONS_APPS_WIFIAPP_H_ */
