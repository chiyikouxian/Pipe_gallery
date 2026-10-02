/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-18     ideapad15s   Sensor debug configuration
 */

#ifndef APPLICATIONS_APPS_SENSOR_DEBUG_CONFIG_H_
#define APPLICATIONS_APPS_SENSOR_DEBUG_CONFIG_H_

/**
 * 传感器调试开关
 *
 * 在逐个调试传感器时，可以关闭其他传感器以减少干扰
 * 调试顺序：
 * 1. 火焰传感器
 * 2. 温湿度传感器（SHT30）
 * 3. 甲烷气体传感器
 * 4. 氧气传感器
 * 5. 位移传感器
 * 6. 电表（三相）
 */

/* ============================================
 * 调试阶段选择（取消注释对应阶段）
 * ============================================ */

/* 阶段1: 火焰传感器调试 */
// #define DEBUG_STAGE_1_FLAME

/* 阶段2: 温湿度传感器调试 */
// #define DEBUG_STAGE_2_SHT30

/* 阶段3: 甲烷传感器调试 */
// #define DEBUG_STAGE_3_METHANE

/* 阶段4: 氧气传感器调试 */
// #define DEBUG_STAGE_4_O2

/* 阶段5: 位移传感器调试 */
// #define DEBUG_STAGE_5_DISPLACEMENT

/* 阶段7: 电表调试 */
// #define DEBUG_STAGE_7_AMMETER

/* 全功能模式：所有传感器同时运行（生产环境） */
#define DEBUG_STAGE_ALL_SENSORS

/* ============================================
 * 根据调试阶段自动配置传感器使能
 * ============================================ */

#if defined(DEBUG_STAGE_1_FLAME)
    /* 阶段1：仅启用火焰传感器 */
    #define ENABLE_FLAME_SENSOR         1
    #define ENABLE_SHT30_SENSOR         0
    #define ENABLE_METHANE_SENSOR       0
    #define ENABLE_O2_SENSOR            0
    #define ENABLE_DISPLACEMENT_SENSOR  0
    #define ENABLE_STRESS_SENSOR        0
    #define ENABLE_AMMETER              0
    #define ENABLE_WIFI_CLOUD           0
    #define ENABLE_LORA                 0
    #define ENABLE_ETHERNET             0

#elif defined(DEBUG_STAGE_2_SHT30)
    /* 阶段2：启用火焰+温湿度 */
    #define ENABLE_FLAME_SENSOR         1
    #define ENABLE_SHT30_SENSOR         1
    #define ENABLE_METHANE_SENSOR       0
    #define ENABLE_O2_SENSOR            0
    #define ENABLE_DISPLACEMENT_SENSOR  0
    #define ENABLE_STRESS_SENSOR        0
    #define ENABLE_AMMETER              0
    #define ENABLE_WIFI_CLOUD           0
    #define ENABLE_LORA                 0
    #define ENABLE_ETHERNET             0

#elif defined(DEBUG_STAGE_3_METHANE)
    /* 阶段3：启用火焰+温湿度+甲烷 */
    #define ENABLE_FLAME_SENSOR         1
    #define ENABLE_SHT30_SENSOR         1
    #define ENABLE_METHANE_SENSOR       1
    #define ENABLE_O2_SENSOR            0
    #define ENABLE_DISPLACEMENT_SENSOR  0
    #define ENABLE_STRESS_SENSOR        0
    #define ENABLE_AMMETER              0
    #define ENABLE_WIFI_CLOUD           0
    #define ENABLE_LORA                 0
    #define ENABLE_ETHERNET             0

#elif defined(DEBUG_STAGE_4_O2)
    /* 阶段4：启用火焰+温湿度+甲烷+氧气 */
    #define ENABLE_FLAME_SENSOR         1
    #define ENABLE_SHT30_SENSOR         1
    #define ENABLE_METHANE_SENSOR       1
    #define ENABLE_O2_SENSOR            1
    #define ENABLE_DISPLACEMENT_SENSOR  0
    #define ENABLE_STRESS_SENSOR        0
    #define ENABLE_AMMETER              0
    #define ENABLE_WIFI_CLOUD           0
    #define ENABLE_LORA                 0
    #define ENABLE_ETHERNET             0

#elif defined(DEBUG_STAGE_5_DISPLACEMENT)
    /* 阶段5：启用所有前置传感器+位移传感器 */
    #define ENABLE_FLAME_SENSOR         1
    #define ENABLE_SHT30_SENSOR         1
    #define ENABLE_METHANE_SENSOR       1
    #define ENABLE_O2_SENSOR            1
    #define ENABLE_DISPLACEMENT_SENSOR  1
    #define ENABLE_STRESS_SENSOR        0
    #define ENABLE_AMMETER              0
    #define ENABLE_WIFI_CLOUD           0
    #define ENABLE_LORA                 0
    #define ENABLE_ETHERNET             0

#elif defined(DEBUG_STAGE_7_AMMETER)
    /* 阶段7：启用所有传感器+电表 */
    #define ENABLE_FLAME_SENSOR         1
    #define ENABLE_SHT30_SENSOR         1
    #define ENABLE_METHANE_SENSOR       1
    #define ENABLE_O2_SENSOR            1
    #define ENABLE_DISPLACEMENT_SENSOR  1
    #define ENABLE_STRESS_SENSOR        1
    #define ENABLE_AMMETER              1
    #define ENABLE_WIFI_CLOUD           0
    #define ENABLE_LORA                 0
    #define ENABLE_ETHERNET             0

#elif defined(DEBUG_STAGE_ALL_SENSORS)
    /* 全功能模式：所有功能启用（生产环境） */
    #define ENABLE_FLAME_SENSOR         1
    #define ENABLE_SHT30_SENSOR         1
    #define ENABLE_METHANE_SENSOR       1
    #define ENABLE_O2_SENSOR            1
    #define ENABLE_DISPLACEMENT_SENSOR  1
    #define ENABLE_STRESS_SENSOR        1
    #define ENABLE_AMMETER              1
    #define ENABLE_WIFI_CLOUD           1
    #define ENABLE_LORA                 1
    #define ENABLE_ETHERNET             1

#else
    /* 默认：启用所有传感器 */
    #define ENABLE_FLAME_SENSOR         1
    #define ENABLE_SHT30_SENSOR         1
    #define ENABLE_METHANE_SENSOR       1
    #define ENABLE_O2_SENSOR            1
    #define ENABLE_DISPLACEMENT_SENSOR  1
    #define ENABLE_STRESS_SENSOR        1
    #define ENABLE_AMMETER              1
    #define ENABLE_WIFI_CLOUD           1
    #define ENABLE_LORA                 1
    #define ENABLE_ETHERNET             1

#endif

/* ============================================
 * 调试输出开关
 * ============================================ */

/* 启用调试输出（会在串口打印详细的传感器数据） */
#define DEBUG_PRINT_FLAME           0   /* 火焰传感器调试输出 */
#define DEBUG_PRINT_SHT30           0   /* SHT30调试输出 */
#define DEBUG_PRINT_METHANE         0   /* 甲烷传感器调试输出 */
#define DEBUG_PRINT_O2              0   /* 氧气传感器调试输出 */
#define DEBUG_PRINT_DISPLACEMENT    0   /* 位移传感器调试输出 */
#define DEBUG_PRINT_AMMETER         0   /* 电表调试输出 */
#define DEBUG_PRINT_MODBUS          0   /* Modbus通信调试输出 */

/* ============================================
 * 调试辅助宏
 * ============================================ */

/* 根据调试阶段打印提示信息 */
#if defined(DEBUG_STAGE_1_FLAME)
    #define DEBUG_STAGE_NAME "Stage 1: Flame Sensor Debug"
#elif defined(DEBUG_STAGE_2_SHT30)
    #define DEBUG_STAGE_NAME "Stage 2: SHT30 Sensor Debug"
#elif defined(DEBUG_STAGE_3_METHANE)
    #define DEBUG_STAGE_NAME "Stage 3: Methane Sensor Debug"
#elif defined(DEBUG_STAGE_4_O2)
    #define DEBUG_STAGE_NAME "Stage 4: O2 Sensor Debug"
#elif defined(DEBUG_STAGE_5_DISPLACEMENT)
    #define DEBUG_STAGE_NAME "Stage 5: Displacement Sensor Debug"
#elif defined(DEBUG_STAGE_7_AMMETER)
    #define DEBUG_STAGE_NAME "Stage 7: Ammeter Debug"
#elif defined(DEBUG_STAGE_ALL_SENSORS)
    #define DEBUG_STAGE_NAME "All Sensors Enabled (Production Mode)"
#else
    #define DEBUG_STAGE_NAME "Default Mode"
#endif

/* 打印当前调试阶段 */
#define PRINT_DEBUG_STAGE() \
    rt_kprintf("\n========================================\n"); \
    rt_kprintf("  %s\n", DEBUG_STAGE_NAME); \
    rt_kprintf("========================================\n\n")

/* ============================================
 * 传感器采样周期配置（单位：ms）
 * ============================================ */

#define FLAME_SAMPLE_PERIOD         500     /* 火焰传感器采样周期 */
#define SHT30_SAMPLE_PERIOD         2000    /* SHT30采样周期 */
#define METHANE_SAMPLE_PERIOD       1000    /* 甲烷传感器采样周期 */
#define O2_SAMPLE_PERIOD            1000    /* 氧气传感器采样周期 */
#define DISPLACEMENT_SAMPLE_PERIOD  500     /* 位移传感器采样周期 */
#define AMMETER_SAMPLE_PERIOD       500     /* 电表采样周期 */

/* ============================================
 * Modbus总线配置
 * ============================================ */

/* Modbus设备优先级（值越小优先级越高） */
#define MODBUS_PRIORITY_DISPLACEMENT  1   /* 位移传感器优先级最高 */
#define MODBUS_PRIORITY_AMMETER       2   /* 电表次之 */

/* Modbus超时配置（单位：ms） */
#define MODBUS_TIMEOUT_DISPLACEMENT   1000  /* 位移传感器超时 */
#define MODBUS_TIMEOUT_AMMETER        1000  /* 电表超时 */

#endif /* APPLICATIONS_APPS_SENSOR_DEBUG_CONFIG_H_ */
