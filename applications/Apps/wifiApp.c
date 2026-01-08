/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-02-24     Administrator       the first version
 */

#include "wifiApp.h"

//上云标志
char task_flag = 0;
//上报消息json
char MQTT_json[128];

void if_is_ok()
{
    if (strstr((const char*)uart4_recived_data,"OK") != NULL) {
        rt_kprintf("字符串匹配：OK\n");
        task_flag++;
    } else if(strstr((const char*)uart4_recived_data,"ERROR") != NULL) {
        rt_kprintf("出错，重试...\n");
        rt_kprintf("字符串：%s\n", uart4_recived_data);
        task_flag = 0;
    } else {
        rt_kprintf("字符串不匹配\n");
        rt_kprintf("字符串：%s\n", uart4_recived_data);
    }
    uart4_buffer_clear();
    rt_kprintf("task_flag：%d\n", task_flag);
}

// 创建一个线程来执行上云操作
void wifi_thread_entry(void *parameter)
{
    char command[512] = {0};
    while(task_flag == 0)
    {
        while(task_flag == 0)       //0.重置8266
        {
            // 重启设备
            uart4_send("AT+RST\r\n");

            // 等待一定时间让串口接收数据
            rt_thread_mdelay(500);
            if_is_ok();
        }
        while(task_flag == 1)       //1.设置station模式
        {
            // 设置为 Station 模式
            uart4_send("AT+CWMODE=1\r\n");

            // 等待一定时间让串口接收数据
            rt_thread_mdelay(500);
            if_is_ok();
        }
        while(task_flag == 2)       //2.连接wifi
        {
            // 连接WiFi
            memset(command, 0, sizeof(command));
            rt_sprintf(command, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
            rt_kprintf("command:%s\n", command);
            uart4_send(command);

            // 等待一定时间让串口接收数据
            rt_thread_mdelay(1000);
            if_is_ok();
        }
        while(task_flag == 3)       //3.设置三元组用户名和密码
        {
            memset(command, 0, sizeof(command));
            rt_sprintf(command, "AT+MQTTUSERCFG=0,1,\"NULL\",\"%s\",\"%s\",0,0,\"\"\r\n", MQTT_USERNAME, MQTT_PASSWORD);
            uart4_send(command);
            rt_kprintf("command:%s\n", command);

            // 等待一定时间让串口接收数据
            rt_thread_mdelay(500);
            if_is_ok();
        }
        while(task_flag == 4)       //4.设置ClientID
        {
            memset(command, 0, sizeof(command));
            rt_sprintf(command, "AT+MQTTCLIENTID=0,\"%s\"\r\n", MQTT_CLIENT_ID);
            uart4_send(command);
            rt_kprintf("command:%s\n", command);

            // 等待一定时间让串口接收数据
            rt_thread_mdelay(1500);
            if_is_ok();
        }
        while(task_flag == 5)       //5.绑定MQTT接入地址
        {
            memset(command, 0, sizeof(command));
            rt_sprintf(command, "AT+MQTTCONN=0,\"%s\",1883,1\r\n", MQTT_IP);
            uart4_send(command);

            // 等待一定时间让串口接收数据
            rt_thread_mdelay(500);
            if_is_ok();
        }
        while(task_flag == 6)       //6.订阅主题
        {
            memset(command, 0, sizeof(command));
            rt_sprintf(command, "AT+MQTTSUB=0,\"oc/devices/%s/sys/properties/report\",1\r\n", MQTT_USERNAME);
            uart4_send(command);

            // 等待一定时间让串口接收数据
            rt_thread_mdelay(500);
            if_is_ok();
        }
        while(task_flag == 7)       //7.消息上报
        {
            memset(command, 0, sizeof(command));
            snprintf(command, sizeof(command), "AT+MQTTPUB=0,\"$oc/devices/%s/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"Node_1\\\"\\,\\\"properties\\\":{\\\"Flow\\\":%.2f\\,\\\"Flame\\\":%d\\,\\\"Methane\\\":%.2f}}]}\",0,0\r\n", MQTT_USERNAME, node[0].Flow, node[0].Flame, node[0].Methane);
            uart4_send(command);
            rt_thread_mdelay(100);
            memset(command, 0, sizeof(command));
            snprintf(command, sizeof(command), "AT+MQTTPUB=0,\"$oc/devices/%s/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"Node_1\\\"\\,\\\"properties\\\":{\\\"current_A\\\":%.2f\\,\\\"current_B\\\":%.2f\\,\\\"current_C\\\":%.2f}}]}\",0,0\r\n", MQTT_USERNAME, node[0].CH1_A[0], node[0].CH1_A[1], node[0].CH1_A[2]);
            uart4_send(command);
            rt_thread_mdelay(100);

//            memset(command, 0, sizeof(command));
//            snprintf(command, sizeof(command), "AT+MQTTPUB=0,\"$oc/devices/%s/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"Node_2\\\"\\,\\\"properties\\\":{\\\"Flow\\\":%.2f\\,\\\"Flame\\\":%d\\,\\\"Methane\\\":%.2f}}]}\",0,0\r\n", MQTT_USERNAME, node[1].Flow, node[1].Flame, node[1].Methane);
//            uart4_send(command);
//            rt_thread_mdelay(100);
//            memset(command, 0, sizeof(command));
//            snprintf(command, sizeof(command), "AT+MQTTPUB=0,\"$oc/devices/%s/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"Node_2\\\"\\,\\\"properties\\\":{\\\"current_A\\\":%.2f\\,\\\"current_B\\\":%.2f\\,\\\"current_C\\\":%.2f}}]}\",0,0\r\n", MQTT_USERNAME, node[1].CH1_A[0], node[1].CH1_A[1], node[1].CH1_A[2]);
//            uart4_send(command);

            // 等待一定时间让串口接收数据
            rt_thread_mdelay(500);
            rt_kprintf("upload data\n");
//            if_is_ok();
        }
    }
}
