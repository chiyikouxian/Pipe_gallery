/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-01     Administrator       the first version
 */
#ifndef APPLICATIONS_APPS_NODEAPP_H_
#define APPLICATIONS_APPS_NODEAPP_H_

typedef struct Node
{
    float CH1_A[3];             //0为AC相电流，1为BC相电流，2为CA相电流
    float Flow;                 //流量
    char Flame;                 //火焰传感器 1有火 0无火
    float Methane;              //甲烷浓度
    float Displacement;         //位移值 (mm)
} Node;

#define NODENUM 2

extern Node node[NODENUM];

#endif /* APPLICATIONS_APPS_NODEAPP_H_ */
