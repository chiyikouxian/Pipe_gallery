/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-08-31     Administrator       Displacement sensor application
 */
#ifndef APPLICATIONS_APPS_DISPLACEMENTSENSORAPP_H_
#define APPLICATIONS_APPS_DISPLACEMENTSENSORAPP_H_

#include "heads.h"

/*************************** Displacement Sensor MODBUS ****************************/
/*
 * 拉绳位移传感器通信参数（基于PDF文档）
 * - 通信协议: Modbus RTU
 * - 波特率: 9600 (默认)
 * - 数据位: 8
 * - 停止位: 1
 * - 校验位: None
 * - 从站地址: 2 (已修改，原默认为0x01)
 */

#define DISPLACEMENT_SLAVE_ADDR         2           /* 位移传感器从站地址 (已修改为2) */
#define DISPLACEMENT_REG_START          0x0000      /* 位移值寄存器起始地址 */
#define DISPLACEMENT_REG_NUM            2           /* 位移值占用2个寄存器(32位浮点数) */

/* 位移传感器功能码 */
#define DISPLACEMENT_FUNC_READ          0x03        /* 读保持寄存器 */
#define DISPLACEMENT_FUNC_WRITE         0x10        /* 写多个寄存器 */

/* 位移传感器寄存器地址定义（基于PDF文档） */
#define DISPLACEMENT_VALUE_REG          0x0000      /* 当前位移值 (16bit integer in Reg[1]) */
#define DISPLACEMENT_SLAVE_ADDR_REG     0x0100      /* 从站地址设置寄存器 */
#define DISPLACEMENT_BAUDRATE_REG       0x0101      /* 波特率设置寄存器 */

/* 位移传感器数据换算系数 */
#define DISPLACEMENT_SCALE_FACTOR       0.025f      /* 寄存器值转换为mm的系数 (1单位 = 0.025mm) */
                                                     /* 实测：1cm = ~400单位，即 1单位 = 0.025mm */
#define DISPLACEMENT_ZERO_OFFSET        74          /* 零点偏移量（原位时的寄存器值） */
                                                     /* 实测：拉线完全收回时 Raw ≈ 74 */

/* 全局变量 */
extern float g_displacement_value;                  /* 当前位移值 (mm 或配置的单位) */
extern rt_bool_t g_displacement_sensor_online;      /* 传感器在线状态 */

/* 线程入口函数 */
void displacement_sensor_test_thread_entry(void *parameter);

/* 工具函数 */
void displacement_print_status(void);

#endif /* APPLICATIONS_APPS_DISPLACEMENTSENSORAPP_H_ */
