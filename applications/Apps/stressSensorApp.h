/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-05     Administrator       GMY400 stress sensor application
 */
#ifndef APPLICATIONS_APPS_STRESSSENSORAPP_H_
#define APPLICATIONS_APPS_STRESSSENSORAPP_H_

#include "heads.h"

/*************************** GMY400 Stress Sensor MODBUS ****************************/
/*
 * GMY400 应力传感器通信参数
 * - 通信协议: Modbus RTU
 * - 波特率: 2400 或 4800 (需要测试确认，默认尝试 2400)
 * - 数据位: 8
 * - 停止位: 1
 * - 校验位: None
 * - 从站地址: 需要确认（默认可能是 1，但需避免与水表冲突，建议设为 3）
 *
 * 接线定义（四芯MHYV电缆）:
 * - 红线: DC18V 电源正极
 * - 白线: 电源 GND 负极
 * - 蓝线: 485A (信号 +)
 * - 绿线: 485B (信号 -)
 *
 * 技术参数:
 * - 量程: 0~400kN
 * - 精度: ±4% FS
 * - 供电: DC18V，工作电流≤60mA
 * - 测量原理: 应变式测力原理
 */

#define STRESS_SLAVE_ADDR           3           /* 应力传感器从站地址 (建议设为3，避免冲突) */
#define STRESS_REG_START            0x0000      /* 应力值寄存器起始地址 (需测试确认) */
#define STRESS_REG_NUM              2           /* 寄存器数量 (假设为2个16位寄存器组成32位数据) */

/* 应力传感器功能码 */
#define STRESS_FUNC_READ            0x03        /* 读保持寄存器 */
#define STRESS_FUNC_WRITE           0x06        /* 写单个寄存器 */

/* 应力传感器寄存器地址定义 (需要根据实际手册确认) */
#define STRESS_VALUE_REG            0x0000      /* 当前应力值寄存器 */
#define STRESS_SLAVE_ADDR_REG       0x0100      /* 从站地址设置寄存器 (假设) */
#define STRESS_BAUDRATE_REG         0x0101      /* 波特率设置寄存器 (假设) */

/* 应力传感器数据换算 */
#define STRESS_RANGE_KN             400.0f      /* 量程: 400kN */
#define STRESS_MAX_RAW              65535       /* 假设16位寄存器最大值 */

/* 全局变量 */
extern float g_stress_value_kn;                 /* 当前应力值 (kN) */
extern rt_bool_t g_stress_sensor_online;        /* 传感器在线状态 */

/* 线程入口函数 */
void stress_sensor_test_thread_entry(void *parameter);

/* 工具函数 */
void stress_print_status(void);

#endif /* APPLICATIONS_APPS_STRESSSENSORAPP_H_ */
