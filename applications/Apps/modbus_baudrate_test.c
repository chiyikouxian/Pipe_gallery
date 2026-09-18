/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-18     ideapad15s   Modbus baudrate testing utility
 */

#include "heads.h"
#include "freeModbusApp.h"
#include "stressSensorApp.h"

/**
 * @brief 临时修改Modbus波特率并测试应力传感器
 *
 * GMY400应力传感器标称波特率为2400或4800，与系统默认9600不匹配
 * 此工具用于快速测试不同波特率下的通信情况
 */

extern eMBMasterReqErrCode eMBMasterReqReadHoldingRegister(UCHAR ucSndAddr,
                                                           USHORT usRegAddr,
                                                           USHORT usNRegs,
                                                           LONG lTimeOut);
extern USHORT usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];

/**
 * @brief 测试指定波特率下的Modbus通信
 */
static void test_modbus_baudrate(uint32_t baudrate, uint8_t slave_addr)
{
    rt_device_t uart_dev;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    eMBMasterReqErrCode error_code;
    uint16_t reg_high, reg_low;

    rt_kprintf("\n========================================\n");
    rt_kprintf("  Testing Modbus @ %u baud\n", baudrate);
    rt_kprintf("  Target: Slave Address %u\n", slave_addr);
    rt_kprintf("========================================\n");

    /* 找到UART3设备 */
    uart_dev = rt_device_find("uart3");
    if (uart_dev == RT_NULL)
    {
        rt_kprintf("[ERROR] Cannot find uart3 device!\n\n");
        return;
    }

    /* 修改波特率 */
    config.baud_rate = baudrate;
    config.data_bits = DATA_BITS_8;
    config.stop_bits = STOP_BITS_1;
    config.parity = PARITY_NONE;

    rt_kprintf("[INFO] Setting UART3 to %u baud...\n", baudrate);

    if (rt_device_control(uart_dev, RT_DEVICE_CTRL_CONFIG, &config) != RT_EOK)
    {
        rt_kprintf("[ERROR] Failed to configure UART3!\n\n");
        return;
    }

    rt_kprintf("[INFO] UART3 configured successfully.\n");
    rt_kprintf("[INFO] Waiting 1 second for bus stabilization...\n");
    rt_thread_mdelay(1000);

    /* 尝试读取保持寄存器 */
    rt_kprintf("[INFO] Reading Holding Register from slave %u...\n", slave_addr);

    error_code = eMBMasterReqReadHoldingRegister(
        slave_addr,
        0x0000,     // 寄存器起始地址
        2,          // 读取2个寄存器
        RT_WAITING_FOREVER
    );

    if (error_code == MB_MRE_NO_ERR)
    {
        reg_high = usMRegHoldBuf[slave_addr - 1][0];
        reg_low  = usMRegHoldBuf[slave_addr - 1][1];

        uint32_t combined = ((uint32_t)reg_high << 16) | reg_low;
        float value = *((float*)&combined);

        rt_kprintf("\n");
        rt_kprintf("✓✓✓ SUCCESS! Device responds at %u baud ✓✓✓\n", baudrate);
        rt_kprintf("\n");
        rt_kprintf("  Raw Data:\n");
        rt_kprintf("    Reg[0] = 0x%04X (%u)\n", reg_high, reg_high);
        rt_kprintf("    Reg[1] = 0x%04X (%u)\n", reg_low, reg_low);
        rt_kprintf("    Combined = 0x%08X (%u)\n", combined, combined);
        rt_kprintf("  Interpreted as:\n");
        rt_kprintf("    Float: %.3f\n", value);
        rt_kprintf("    Stress (0-400kN): %.2f kN\n", (combined / 65535.0f) * 400.0f);
        rt_kprintf("\n");
    }
    else
    {
        rt_kprintf("\n");
        rt_kprintf("✗✗✗ FAILED - No response (Error: %d) ✗✗✗\n", error_code);
        rt_kprintf("\n");
    }

    rt_kprintf("========================================\n\n");
}

/**
 * @brief MSH命令：测试应力传感器的波特率
 * 用法: stress_baudrate_scan
 */
static void stress_baudrate_scan(int argc, char **argv)
{
    uint32_t baudrates[] = {2400, 4800, 9600, 19200};
    uint8_t slave_addr = STRESS_SLAVE_ADDR;  // 默认地址3

    if (argc == 2)
    {
        slave_addr = atoi(argv[1]);
        if (slave_addr < 1 || slave_addr > 247)
        {
            rt_kprintf("\nError: Slave address must be 1-247\n\n");
            return;
        }
    }

    rt_kprintf("\n");
    rt_kprintf("╔════════════════════════════════════════╗\n");
    rt_kprintf("║  GMY400 Stress Sensor Baudrate Scan  ║\n");
    rt_kprintf("╚════════════════════════════════════════╝\n");
    rt_kprintf("\n");
    rt_kprintf("  This will test common baudrates:\n");
    rt_kprintf("  - 2400 baud (GMY400 default)\n");
    rt_kprintf("  - 4800 baud (GMY400 alternative)\n");
    rt_kprintf("  - 9600 baud (current system setting)\n");
    rt_kprintf("  - 19200 baud (for reference)\n");
    rt_kprintf("\n");
    rt_kprintf("  ⚠️  WARNING: This will temporarily stop\n");
    rt_kprintf("      normal Modbus communication!\n");
    rt_kprintf("\n");
    rt_kprintf("  Target slave address: %u\n", slave_addr);
    rt_kprintf("\n");
    rt_kprintf("  Estimated time: ~20 seconds\n");
    rt_kprintf("\n");

    /* 暂停正常的Modbus轮询（如果可能） */
    rt_kprintf("[WARN] Please ensure Modbus threads are idle!\n");
    rt_kprintf("[INFO] Starting baudrate scan in 3 seconds...\n");
    rt_thread_mdelay(3000);

    /* 测试各个波特率 */
    for (int i = 0; i < sizeof(baudrates) / sizeof(baudrates[0]); i++)
    {
        test_modbus_baudrate(baudrates[i], slave_addr);
        rt_thread_mdelay(1000);  // 每次测试间隔1秒
    }

    /* 恢复默认波特率 */
    rt_kprintf("[INFO] Restoring default baudrate (9600)...\n");
    test_modbus_baudrate(9600, slave_addr);

    rt_kprintf("\n");
    rt_kprintf("╔════════════════════════════════════════╗\n");
    rt_kprintf("║         Baudrate Scan Complete        ║\n");
    rt_kprintf("╚════════════════════════════════════════╝\n");
    rt_kprintf("\n");
    rt_kprintf("Next steps:\n");
    rt_kprintf("1. If 2400 or 4800 worked, modify PORT_BAUDRATE\n");
    rt_kprintf("   in freeModbusApp.h and recompile.\n");
    rt_kprintf("2. If none worked, check:\n");
    rt_kprintf("   - RS485 wiring (485A/485B)\n");
    rt_kprintf("   - Power supply (DC18V)\n");
    rt_kprintf("   - Slave address jumpers\n");
    rt_kprintf("\n");
}
MSH_CMD_EXPORT(stress_baudrate_scan, Scan baudrates for GMY400 stress sensor);

/**
 * @brief MSH命令：测试单个波特率
 * 用法: test_baudrate <baudrate> [slave_addr]
 * 示例: test_baudrate 2400 3
 */
static void test_baudrate(int argc, char **argv)
{
    uint32_t baudrate;
    uint8_t slave_addr = STRESS_SLAVE_ADDR;

    if (argc < 2 || argc > 3)
    {
        rt_kprintf("\nUsage: test_baudrate <baudrate> [slave_addr]\n");
        rt_kprintf("Example: test_baudrate 2400 3\n\n");
        return;
    }

    baudrate = atoi(argv[1]);

    if (baudrate < 300 || baudrate > 115200)
    {
        rt_kprintf("\nError: Invalid baudrate (300-115200)\n\n");
        return;
    }

    if (argc == 3)
    {
        slave_addr = atoi(argv[2]);
        if (slave_addr < 1 || slave_addr > 247)
        {
            rt_kprintf("\nError: Slave address must be 1-247\n\n");
            return;
        }
    }

    test_modbus_baudrate(baudrate, slave_addr);
}
MSH_CMD_EXPORT(test_baudrate, Test specific Modbus baudrate);
