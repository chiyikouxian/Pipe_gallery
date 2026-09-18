/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-18     ideapad15s   Debug MSH commands for sensor testing
 */

#include "heads.h"
#include "linesensor.h"
#include "MethaneSensorApp.h"
#include "sht30App.h"
#include "o2SensorApp.h"
#include "freeModbusApp.h"
#include "stressSensorApp.h"
#include "huaweiCloudApp.h"
#include "stm32h7xx_hal.h"

/**
 * @brief 显示所有传感器数据汇总
 */
static void show_all_sensors(int argc, char **argv)
{
    rt_kprintf("\n");
    rt_kprintf("========================================\n");
    rt_kprintf("  Pipe Gallery Node - Sensor Status\n");
    rt_kprintf("========================================\n");
    rt_kprintf("\n");

    /* 1. 火焰传感器 (Five-channel ADC) */
    rt_uint8_t fire_count = 0;
    rt_kprintf("1. Flame Sensors (5-channel ADC):\n");
    rt_kprintf("   A1 (ADC0): %5u    ", g_adc_ch0);
    if (g_adc_ch0 > 60000) {
        rt_kprintf("[FIRE DETECTED!]\n");
        fire_count++;
    } else {
        rt_kprintf("[Normal]\n");
    }

    rt_kprintf("   A2 (ADC1): %5u    ", g_adc_ch1);
    if (g_adc_ch1 > 60000) {
        rt_kprintf("[FIRE DETECTED!]\n");
        fire_count++;
    } else {
        rt_kprintf("[Normal]\n");
    }

    rt_kprintf("   A3 (ADC3): %5u    ", g_adc_ch3);
    if (g_adc_ch3 > 60000) {
        rt_kprintf("[FIRE DETECTED!]\n");
        fire_count++;
    } else {
        rt_kprintf("[Normal]\n");
    }

    rt_kprintf("   A4 (ADC4): %5u    ", g_adc_ch4);
    if (g_adc_ch4 > 60000) {
        rt_kprintf("[FIRE DETECTED!]\n");
        fire_count++;
    } else {
        rt_kprintf("[Normal]\n");
    }

    rt_kprintf("   A5 (ADC5): %5u    ", g_adc_ch5);
    if (g_adc_ch5 > 60000) {
        rt_kprintf("[FIRE DETECTED!]\n");
        fire_count++;
    } else {
        rt_kprintf("[Normal]\n");
    }

    rt_kprintf("   >> Overall: ");
    if (fire_count >= 4) {
        rt_kprintf("**FIRE ALARM** (%d/5 triggered)\n", fire_count);
    } else {
        rt_kprintf("Normal (%d/5 triggered)\n", fire_count);
    }
    rt_kprintf("\n");

    /* 2. 温湿度传感器 SHT30 */
    rt_kprintf("2. SHT30 Temperature & Humidity:\n");
    int temp_int = (int)g_temperature_c;
    int temp_dec = (int)((g_temperature_c - temp_int) * 100);
    if (temp_dec < 0) temp_dec = -temp_dec;

    int humi_int = (int)g_humidity_rh;
    int humi_dec = (int)((g_humidity_rh - humi_int) * 100);
    if (humi_dec < 0) humi_dec = -humi_dec;

    rt_kprintf("   Temperature: %d.%02d °C\n", temp_int, temp_dec);
    rt_kprintf("   Humidity:    %d.%02d %%RH\n", humi_int, humi_dec);
    rt_kprintf("\n");

    /* 3. 甲烷传感器 */
    rt_kprintf("3. Methane Gas Sensor:\n");
    rt_kprintf("   Concentration: %u ppm", g_methane_ppm);
    if (g_methane_ppm > 10000)
        rt_kprintf("  [DANGER!]\n");
    else if (g_methane_ppm > 5000)
        rt_kprintf("  [WARNING!]\n");
    else
        rt_kprintf("  [Normal]\n");

    rt_kprintf("   LEL%%:          %u %%\n", g_methane_lel);
    rt_kprintf("\n");

    /* 4. 氧气传感器 */
    rt_kprintf("4. O2 Sensor:\n");
    int o2_int = (int)g_o2_concentration;
    int o2_dec = (int)((g_o2_concentration - o2_int) * 10);
    if (o2_dec < 0) o2_dec = -o2_dec;

    rt_kprintf("   O2: %d.%d %%", o2_int, o2_dec);
    if (g_o2_concentration < 19.5f)
        rt_kprintf("  [LOW OXYGEN!]\n");
    else if (g_o2_concentration > 23.5f)
        rt_kprintf("  [HIGH OXYGEN!]\n");
    else
        rt_kprintf("  [Normal]\n");
    rt_kprintf("\n");

    /* 5. 位移传感器 */
    rt_kprintf("5. Displacement Sensor (Modbus):\n");
    int disp_int = (int)Displacement;
    int disp_dec = (int)((Displacement - disp_int) * 1000);
    if (disp_dec < 0) disp_dec = -disp_dec;
    rt_kprintf("   Displacement: %d.%03d mm\n", disp_int, disp_dec);
    rt_kprintf("\n");

    /* 6. 应力传感器 */
    rt_kprintf("6. Stress Sensor GMY400 (Modbus):\n");
    if (g_stress_sensor_online)
    {
        int stress_int = (int)g_stress_value_kn;
        int stress_dec = (int)((g_stress_value_kn - stress_int) * 100);
        if (stress_dec < 0) stress_dec = -stress_dec;
        rt_kprintf("   Stress: %d.%02d kN  [Online]\n", stress_int, stress_dec);
    }
    else
    {
        rt_kprintf("   Stress: ---.-- kN  [Offline]\n");
    }
    rt_kprintf("\n");

    /* 7. 电表（三相电流） */
    rt_kprintf("7. Ammeter (3-Phase Current):\n");
    for (int i = 0; i < 3; i++)
    {
        int curr_int = (int)Current[i];
        int curr_dec = (int)((Current[i] - curr_int) * 100);
        if (curr_dec < 0) curr_dec = -curr_dec;
        rt_kprintf("   Phase %d Current: %d.%02d A\n", i, curr_int, curr_dec);
    }
    rt_kprintf("\n");

    /* 8. 电表（三相电压） */
    rt_kprintf("8. Ammeter (3-Phase Voltage):\n");
    for (int i = 0; i < 3; i++)
    {
        int volt_int = (int)Voltage[i];
        int volt_dec = (int)((Voltage[i] - volt_int) * 100);
        if (volt_dec < 0) volt_dec = -volt_dec;
        rt_kprintf("   Phase %d Voltage: %d.%02d V\n", i, volt_int, volt_dec);
    }

    rt_kprintf("\n========================================\n");
    rt_kprintf("  Use 'sensors' to refresh this display\n");
    rt_kprintf("========================================\n\n");
}
MSH_CMD_EXPORT_ALIAS(show_all_sensors, sensors, Show all sensor data);

/**
 * @brief 显示火焰传感器详细信息
 */
static void show_flame(int argc, char **argv)
{
    rt_uint8_t fire_count = 0;

    rt_kprintf("\n========================================\n");
    rt_kprintf("  Five-Channel Flame Sensor Status\n");
    rt_kprintf("========================================\n");
    rt_kprintf("  Channel  |  ADC Value  |  Status\n");
    rt_kprintf("-----------|-------------|-------------\n");

    rt_kprintf("  A1 (CH0) |    %5u    | ", g_adc_ch0);
    if (g_adc_ch0 > 60000) {
        rt_kprintf("FIRE!\n");
        fire_count++;
    } else {
        rt_kprintf("Normal\n");
    }

    rt_kprintf("  A2 (CH1) |    %5u    | ", g_adc_ch1);
    if (g_adc_ch1 > 60000) {
        rt_kprintf("FIRE!\n");
        fire_count++;
    } else {
        rt_kprintf("Normal\n");
    }

    rt_kprintf("  A3 (CH3) |    %5u    | ", g_adc_ch3);
    if (g_adc_ch3 > 60000) {
        rt_kprintf("FIRE!\n");
        fire_count++;
    } else {
        rt_kprintf("Normal\n");
    }

    rt_kprintf("  A4 (CH4) |    %5u    | ", g_adc_ch4);
    if (g_adc_ch4 > 60000) {
        rt_kprintf("FIRE!\n");
        fire_count++;
    } else {
        rt_kprintf("Normal\n");
    }

    rt_kprintf("  A5 (CH5) |    %5u    | ", g_adc_ch5);
    if (g_adc_ch5 > 60000) {
        rt_kprintf("FIRE!\n");
        fire_count++;
    } else {
        rt_kprintf("Normal\n");
    }

    rt_kprintf("========================================\n");
    rt_kprintf("  Fire Channels: %d/5\n", fire_count);
    rt_kprintf("  Overall Status: ");
    if (fire_count >= 4) {
        rt_kprintf("**FIRE ALARM**\n");
    } else {
        rt_kprintf("Normal\n");
    }
    rt_kprintf("========================================\n");
    rt_kprintf("  ADC Range: 0-65535 (16-bit)\n");
    rt_kprintf("  Threshold: >60000 = Fire Detected\n");
    rt_kprintf("  Alarm Rule: >= 4 channels triggered\n");
    rt_kprintf("========================================\n\n");
}
MSH_CMD_EXPORT_ALIAS(show_flame, flame, Show flame sensor status);

/**
 * @brief 显示环境传感器（温湿度+氧气+甲烷）
 */
static void show_environment(int argc, char **argv)
{
    rt_kprintf("\n========================================\n");
    rt_kprintf("  Environment Sensors\n");
    rt_kprintf("========================================\n");

    /* 温度 */
    int temp_int = (int)g_temperature_c;
    int temp_dec = (int)((g_temperature_c - temp_int) * 100);
    if (temp_dec < 0) temp_dec = -temp_dec;
    rt_kprintf("  Temperature: %d.%02d °C\n", temp_int, temp_dec);

    /* 湿度 */
    int humi_int = (int)g_humidity_rh;
    int humi_dec = (int)((g_humidity_rh - humi_int) * 100);
    if (humi_dec < 0) humi_dec = -humi_dec;
    rt_kprintf("  Humidity:    %d.%02d %%RH\n", humi_int, humi_dec);

    /* 氧气 */
    int o2_int = (int)g_o2_concentration;
    int o2_dec = (int)((g_o2_concentration - o2_int) * 10);
    if (o2_dec < 0) o2_dec = -o2_dec;
    rt_kprintf("  O2:          %d.%d %%", o2_int, o2_dec);
    if (g_o2_concentration < 19.5f)
        rt_kprintf("  [LOW!]\n");
    else if (g_o2_concentration > 23.5f)
        rt_kprintf("  [HIGH!]\n");
    else
        rt_kprintf("\n");

    /* 甲烷 */
    rt_kprintf("  Methane:     %u ppm", g_methane_ppm);
    if (g_methane_ppm > 10000)
        rt_kprintf("  [DANGER!]\n");
    else if (g_methane_ppm > 5000)
        rt_kprintf("  [WARNING!]\n");
    else
        rt_kprintf("\n");

    rt_kprintf("  Methane LEL: %u %%\n", g_methane_lel);

    rt_kprintf("========================================\n\n");
}
MSH_CMD_EXPORT_ALIAS(show_environment, env, Show environment sensors);

/**
 * @brief 显示Modbus设备状态
 */
static void show_modbus(int argc, char **argv)
{
    rt_kprintf("\n========================================\n");
    rt_kprintf("  Modbus RTU Devices (UART3)\n");
    rt_kprintf("========================================\n");
    rt_kprintf("  Baudrate: 9600\n");
    rt_kprintf("  Parity:   None\n");
    rt_kprintf("========================================\n\n");

    /* 位移传感器 */
    rt_kprintf("1. Displacement Sensor:\n");
    rt_kprintf("   Slave Address: 1\n");
    int disp_int = (int)Displacement;
    int disp_dec = (int)((Displacement - disp_int) * 1000);
    if (disp_dec < 0) disp_dec = -disp_dec;
    rt_kprintf("   Value: %d.%03d mm\n\n", disp_int, disp_dec);

    /* 应力传感器 */
    rt_kprintf("2. Stress Sensor GMY400:\n");
    rt_kprintf("   Slave Address: 3\n");
    if (g_stress_sensor_online)
    {
        int stress_int = (int)g_stress_value_kn;
        int stress_dec = (int)((g_stress_value_kn - stress_int) * 100);
        if (stress_dec < 0) stress_dec = -stress_dec;
        rt_kprintf("   Value: %d.%02d kN\n", stress_int, stress_dec);
        rt_kprintf("   Status: Online\n\n");
    }
    else
    {
        rt_kprintf("   Value: ---.-- kN\n");
        rt_kprintf("   Status: Offline\n\n");
    }

    /* 电表 */
    rt_kprintf("3. Ammeter (3-Phase):\n");
    rt_kprintf("   Slave Address: 73\n");
    rt_kprintf("   Voltage:\n");
    for (int i = 0; i < 3; i++)
    {
        int volt_int = (int)Voltage[i];
        int volt_dec = (int)((Voltage[i] - volt_int) * 100);
        if (volt_dec < 0) volt_dec = -volt_dec;
        rt_kprintf("     Phase %d: %d.%02d V\n", i, volt_int, volt_dec);
    }
    rt_kprintf("   Current:\n");
    for (int i = 0; i < 3; i++)
    {
        int curr_int = (int)Current[i];
        int curr_dec = (int)((Current[i] - curr_int) * 100);
        if (curr_dec < 0) curr_dec = -curr_dec;
        rt_kprintf("     Phase %d: %d.%02d A\n", i, curr_int, curr_dec);
    }

    rt_kprintf("\n========================================\n");
    rt_kprintf("  Use 'stress_scan_bus' to scan devices\n");
    rt_kprintf("========================================\n\n");
}
MSH_CMD_EXPORT_ALIAS(show_modbus, modbus, Show Modbus device status);

/**
 * @brief 持续监控传感器数据（按Ctrl+C退出）
 */
static void monitor_sensors(int argc, char **argv)
{
    int interval = 1000; /* 默认1秒 */

    if (argc == 2)
    {
        interval = atoi(argv[1]) * 1000;
        if (interval < 100) interval = 100;
        if (interval > 10000) interval = 10000;
    }

    rt_kprintf("\n========================================\n");
    rt_kprintf("  Sensor Monitoring (Press Ctrl+C to stop)\n");
    rt_kprintf("  Refresh interval: %d ms\n", interval);
    rt_kprintf("========================================\n\n");

    while (1)
    {
        rt_kprintf("\033[2J\033[H"); /* Clear screen and move cursor to top-left */

        rt_kprintf("========================================\n");
        rt_kprintf("  Real-Time Sensor Monitoring\n");
        rt_kprintf("========================================\n\n");

        /* 火焰 */
        rt_kprintf("Flame: A1=%4u A2=%4u A3=%4u A4=%4u A5=%4u\n",
                   g_adc_ch0, g_adc_ch1, g_adc_ch3, g_adc_ch4, g_adc_ch5);

        /* 温湿度 */
        int temp_int = (int)g_temperature_c;
        int temp_dec = (int)((g_temperature_c - temp_int) * 100);
        if (temp_dec < 0) temp_dec = -temp_dec;

        int humi_int = (int)g_humidity_rh;
        int humi_dec = (int)((g_humidity_rh - humi_int) * 100);
        if (humi_dec < 0) humi_dec = -humi_dec;

        rt_kprintf("Temp: %d.%02d°C  Humidity: %d.%02d%%RH\n",
                   temp_int, temp_dec, humi_int, humi_dec);

        /* 氧气+甲烷 */
        int o2_int = (int)g_o2_concentration;
        int o2_dec = (int)((g_o2_concentration - o2_int) * 10);
        if (o2_dec < 0) o2_dec = -o2_dec;

        rt_kprintf("O2: %d.%d%%  Methane: %uppm (%u%%LEL)\n",
                   o2_int, o2_dec, g_methane_ppm, g_methane_lel);

        /* 位移+应力 */
        int disp_int = (int)Displacement;
        int disp_dec = (int)((Displacement - disp_int) * 1000);
        if (disp_dec < 0) disp_dec = -disp_dec;

        rt_kprintf("Displacement: %d.%03dmm  ", disp_int, disp_dec);

        if (g_stress_sensor_online)
        {
            int stress_int = (int)g_stress_value_kn;
            int stress_dec = (int)((g_stress_value_kn - stress_int) * 100);
            if (stress_dec < 0) stress_dec = -stress_dec;
            rt_kprintf("Stress: %d.%02dkN\n", stress_int, stress_dec);
        }
        else
        {
            rt_kprintf("Stress: Offline\n");
        }

        /* 电流 */
        int curr0 = (int)Current[0];
        int curr1 = (int)Current[1];
        int curr2 = (int)Current[2];
        rt_kprintf("Current: P0=%dA P1=%dA P2=%dA\n", curr0, curr1, curr2);

        rt_kprintf("\n[Press Ctrl+C to stop monitoring]\n");

        rt_thread_mdelay(interval);
    }
}
MSH_CMD_EXPORT_ALIAS(monitor_sensors, monitor, Monitor sensors in real-time);

/**
 * @brief 测试单个传感器
 */
static void test_sensor(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("\nUsage: test <sensor_name>\n");
        rt_kprintf("\nAvailable sensors:\n");
        rt_kprintf("  flame       - Five-channel flame sensor\n");
        rt_kprintf("  sht30       - Temperature & humidity sensor\n");
        rt_kprintf("  methane     - Methane gas sensor\n");
        rt_kprintf("  o2          - Oxygen sensor\n");
        rt_kprintf("  displacement - Displacement sensor\n");
        rt_kprintf("  stress      - Stress sensor GMY400\n");
        rt_kprintf("  ammeter     - 3-phase ammeter\n");
        rt_kprintf("\nExample: test flame\n\n");
        return;
    }

    const char *sensor = argv[1];

    if (rt_strcmp(sensor, "flame") == 0)
    {
        show_flame(0, NULL);
    }
    else if (rt_strcmp(sensor, "sht30") == 0)
    {
        rt_kprintf("\n[SHT30] Temperature: %.2f°C, Humidity: %.2f%%RH\n\n",
                   g_temperature_c, g_humidity_rh);
    }
    else if (rt_strcmp(sensor, "methane") == 0)
    {
        if (!g_methane_data_valid)
        {
            rt_kprintf("\n[Methane] No valid GM-402B frame received\n\n");
        }
        else
        {
            rt_kprintf("\n[Methane] Concentration: %uppm, LEL: %u%%, Status: 0x%02X%s\n\n",
                       g_methane_ppm, g_methane_lel, g_methane_status,
                       (g_methane_status == 0xAA) ? " (Normal)" : " (Check sensor)");
        }
    }
    else if (rt_strcmp(sensor, "o2") == 0)
    {
        rt_kprintf("\n[O2] Concentration: %.1f%%\n\n", g_o2_concentration);
    }
    else if (rt_strcmp(sensor, "displacement") == 0)
    {
        if (!g_displacement_sensor_online)
        {
            rt_kprintf("\n[Displacement] Offline - no valid Modbus response\n\n");
            return;
        }

        /* rt_kprintf in this build has no floating-point formatter.  Print
         * the value as integer millimetres plus a three-digit fraction. */
        int disp_int = (int)Displacement;
        int disp_dec = (int)((Displacement - disp_int) * 1000.0f);
        if (disp_dec < 0) disp_dec = -disp_dec;
        rt_kprintf("\n[Displacement] Value: %d.%03dmm\n\n", disp_int, disp_dec);
    }
    else if (rt_strcmp(sensor, "stress") == 0)
    {
        if (g_stress_sensor_online)
            rt_kprintf("\n[Stress] Value: %.2fkN [Online]\n\n", g_stress_value_kn);
        else
            rt_kprintf("\n[Stress] Offline\n\n");
    }
    else if (rt_strcmp(sensor, "ammeter") == 0)
    {
        rt_kprintf("\n[Ammeter]\n");
        rt_kprintf("  Voltage: P0=%.2fV P1=%.2fV P2=%.2fV\n",
                   Voltage[0], Voltage[1], Voltage[2]);
        rt_kprintf("  Current: P0=%.2fA P1=%.2fA P2=%.2fA\n\n",
                   Current[0], Current[1], Current[2]);
    }
    else
    {
        rt_kprintf("\nError: Unknown sensor '%s'\n", sensor);
        rt_kprintf("Use 'test' without arguments to see available sensors.\n\n");
    }
}
MSH_CMD_EXPORT_ALIAS(test_sensor, test, Test individual sensor);

/**
 * @brief 显示 UART 引脚配置信息 - 已禁用，用于单独测试
 */
#if 0
static void show_uart_pins(int argc, char **argv)
{
    rt_kprintf("\n========================================\n");
    rt_kprintf("  UART Pin Configuration\n");
    rt_kprintf("========================================\n");
    rt_kprintf("\n");

    rt_kprintf("UART1 (Console):\n");
    rt_kprintf("  TX: PA9\n");
    rt_kprintf("  RX: PA10\n");
    rt_kprintf("\n");

    rt_kprintf("UART2 (Methane Sensor GM-402B):\n");
    rt_kprintf("  TX: PD5 (MCU -> Sensor RX)\n");
    rt_kprintf("  RX: PD6 (MCU <- Sensor TX)\n");
    rt_kprintf("  Baud: 19200\n");
    rt_kprintf("  ** IMPORTANT: PA2/PA3 are NOT used! **\n");
    rt_kprintf("  ** PA2 is used by ETH_MDIO **\n");
    rt_kprintf("  ** PA3 is used by ADC (Flame A2) **\n");
    rt_kprintf("\n");

    rt_kprintf("UART3 (Modbus RTU):\n");
    rt_kprintf("  TX: PB10\n");
    rt_kprintf("  RX: PB11\n");
    rt_kprintf("  Baud: 9600\n");
    rt_kprintf("  Devices: Displacement, Stress, Ammeter\n");
    rt_kprintf("\n");

    rt_kprintf("UART4:\n");
    rt_kprintf("  TX: PA12\n");
    rt_kprintf("  RX: PA11\n");
    rt_kprintf("\n");

    rt_kprintf("UART5:\n");
    rt_kprintf("  TX: PB6\n");
    rt_kprintf("  RX: PB5\n");
    rt_kprintf("\n");

    rt_kprintf("========================================\n");
    rt_kprintf("Hardware Connection Checklist:\n");
    rt_kprintf("========================================\n");
    rt_kprintf("[ ] GM-402B TX -> MCU PD6 (UART2_RX)\n");
    rt_kprintf("[ ] GM-402B RX -> MCU PD5 (UART2_TX)\n");
    rt_kprintf("[ ] GM-402B GND -> MCU GND\n");
    rt_kprintf("[ ] GM-402B VIN -> 3.3V or 5V\n");
    rt_kprintf("[ ] Verify sensor power LED is ON\n");
    rt_kprintf("========================================\n\n");
}
#endif
/* MSH_CMD_EXPORT_ALIAS(show_uart_pins, uart_pins, Show UART pin configuration); */

/**
 * @brief 检查 UART2 实际的引脚配置（从寄存器读取）- 已禁用，用于单独测试
 */
#if 0
static void check_uart2_pins(int argc, char **argv)
{
    rt_kprintf("\n========================================\n");
    rt_kprintf("  UART2 Actual Pin Configuration\n");
    rt_kprintf("========================================\n");
    rt_kprintf("\n");

    /* USART2 寄存器基址 */
    USART_TypeDef *uart2 = USART2;

    rt_kprintf("USART2 Hardware Info:\n");
    rt_kprintf("  Base Address: 0x%08X\n", (unsigned int)uart2);
    rt_kprintf("  CR1 Register: 0x%08X\n", (unsigned int)uart2->CR1);
    rt_kprintf("  Enabled: %s\n", (uart2->CR1 & USART_CR1_UE) ? "YES" : "NO");
    rt_kprintf("  TX Enabled: %s\n", (uart2->CR1 & USART_CR1_TE) ? "YES" : "NO");
    rt_kprintf("  RX Enabled: %s\n", (uart2->CR1 & USART_CR1_RE) ? "YES" : "NO");
    rt_kprintf("\n");

    rt_kprintf("Expected Pin Configuration:\n");
    rt_kprintf("  TX: PD5 (GPIOD Pin 5)\n");
    rt_kprintf("  RX: PD6 (GPIOD Pin 6)\n");
    rt_kprintf("\n");

    /* 检查 GPIOD 的 AFR (Alternate Function Register) */
    GPIO_TypeDef *gpiod = GPIOD;
    rt_kprintf("GPIOD Actual Configuration:\n");

    /* PD5 配置 */
    rt_uint32_t moder5 = (gpiod->MODER >> (5 * 2)) & 0x3;
    rt_uint32_t afr5 = (gpiod->AFR[0] >> (5 * 4)) & 0xF;
    rt_kprintf("  PD5 (UART2_TX):\n");
    rt_kprintf("    Mode: 0x%X ", moder5);
    if (moder5 == 0) rt_kprintf("(Input)");
    else if (moder5 == 1) rt_kprintf("(Output)");
    else if (moder5 == 2) rt_kprintf("(Alternate Function)");
    else if (moder5 == 3) rt_kprintf("(Analog)");
    rt_kprintf("\n");
    rt_kprintf("    AF: AF%u", afr5);
    if (afr5 == 7) rt_kprintf(" (USART2 - Correct!)");
    else rt_kprintf(" (Wrong! Should be AF7)");
    rt_kprintf("\n");

    /* PD6 配置 */
    rt_uint32_t moder6 = (gpiod->MODER >> (6 * 2)) & 0x3;
    rt_uint32_t afr6 = (gpiod->AFR[0] >> (6 * 4)) & 0xF;
    rt_kprintf("  PD6 (UART2_RX):\n");
    rt_kprintf("    Mode: 0x%X ", moder6);
    if (moder6 == 0) rt_kprintf("(Input)");
    else if (moder6 == 1) rt_kprintf("(Output)");
    else if (moder6 == 2) rt_kprintf("(Alternate Function)");
    else if (moder6 == 3) rt_kprintf("(Analog)");
    rt_kprintf("\n");
    rt_kprintf("    AF: AF%u", afr6);
    if (afr6 == 7) rt_kprintf(" (USART2 - Correct!)");
    else rt_kprintf(" (Wrong! Should be AF7)");
    rt_kprintf("\n");

    rt_kprintf("\n");
    rt_kprintf("Diagnosis:\n");
    if (moder5 == 2 && afr5 == 7 && moder6 == 2 && afr6 == 7)
    {
        rt_kprintf("  ✓ UART2 is correctly mapped to PD5/PD6\n");
    }
    else
    {
        rt_kprintf("  ✗ UART2 pin configuration is WRONG!\n");
        if (moder5 != 2 || afr5 != 7)
            rt_kprintf("  ✗ PD5 is not configured as UART2_TX\n");
        if (moder6 != 2 || afr6 != 7)
            rt_kprintf("  ✗ PD6 is not configured as UART2_RX\n");
    }

    rt_kprintf("\n========================================\n\n");
}
#endif
/* MSH_CMD_EXPORT_ALIAS(check_uart2_pins, check_uart2, Check UART2 actual pin configuration from registers); */

/**
 * @brief O2 sensor debug command - shows raw ADC and O2 concentration
 */
static void o2_debug(int argc, char **argv)
{
    rt_adc_device_t adc_dev;
    rt_uint32_t raw;
    rt_int32_t o2_x10;
    float o2_percent;

    rt_kprintf("\n========================================\n");
    rt_kprintf("  O2 Sensor Debug (ADC1 Channel 18)\n");
    rt_kprintf("========================================\n\n");

    /* Find ADC device */
    adc_dev = (rt_adc_device_t)rt_device_find("adc1");
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("ERROR: Cannot find adc1 device!\n\n");
        return;
    }

    /* Enable ADC */
    rt_adc_enable(adc_dev, 18);

    /* Read raw ADC value */
    raw = rt_adc_read(adc_dev, 18);

    /* Convert to O2 percentage (x10) using ADC raw value directly */
    o2_x10 = (rt_int32_t)(raw - 0) * 209 / (2410 - 0);

    /* Air-stabilize */
    if (o2_x10 >= 207 && o2_x10 <= 211)
    {
        o2_x10 = 209;
    }

    o2_percent = (float)o2_x10 / 10.0f;

    rt_kprintf("Raw ADC Value:     %u (0-65535)\n", raw);
    rt_kprintf("O2 Concentration:  %.1f %%\n", o2_percent);
    rt_kprintf("\n");
    rt_kprintf("Calibration Parameters:\n");
    rt_kprintf("  Zero Offset:     0 (ADC raw value)\n");
    rt_kprintf("  Full Scale:      2410 (ADC raw at 20.9%% O2)\n");
    rt_kprintf("  Stabilize Range: 20.7%% - 21.1%% -> 20.9%%\n");
    rt_kprintf("\n");
    rt_kprintf("Expected Values:\n");
    rt_kprintf("  Normal Air:      ADC ~2410, O2 ~20.9%%\n");
    rt_kprintf("  Low O2 Warning:  <19.5%%\n");
    rt_kprintf("  High O2 Warning: >23.5%%\n");
    rt_kprintf("\n");

    if (raw < 500)
    {
        rt_kprintf("WARNING: ADC value very low!\n");
        rt_kprintf("  Check sensor power and connections.\n");
    }
    else if (o2_percent < 10.0f || o2_percent > 30.0f)
    {
        rt_kprintf("WARNING: O2 value out of normal range!\n");
        rt_kprintf("  Possible causes:\n");
        rt_kprintf("  1. Sensor needs calibration (adjust O2_FULL_SCALE_ADC)\n");
        rt_kprintf("  2. Current calibration: ADC 2410 = 20.9%% O2\n");
        rt_kprintf("  3. Your ADC: %u -> %.1f%% O2\n", raw, o2_percent);
    }

    rt_kprintf("========================================\n\n");

    rt_adc_disable(adc_dev, 18);
}
MSH_CMD_EXPORT_ALIAS(o2_debug, o2_debug, Debug O2 sensor raw ADC and concentration);
