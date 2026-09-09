/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-05     Administrator       GMY400 stress sensor application
 */

#include "stressSensorApp.h"
#include "freeModbusApp.h"

/* 全局变量定义 */
float g_stress_value_kn = 0.0f;
rt_bool_t g_stress_sensor_online = RT_FALSE;

/**
 * @brief 打印应力传感器状态信息
 */
void stress_print_status(void)
{
    if (g_stress_sensor_online)
    {
        int stress_int = (int)g_stress_value_kn;
        int stress_dec = (int)((g_stress_value_kn - stress_int) * 100);
        if (stress_dec < 0) stress_dec = -stress_dec;

        rt_kprintf("[Stress] Online - Value: %d.%02d kN\n", stress_int, stress_dec);
    }
    else
    {
        rt_kprintf("[Stress] Offline - No communication\n");
    }
}

/**
 * @brief 应力传感器测试线程入口
 *
 * 该线程用于独立测试应力传感器的Modbus通信
 * 在确认通信正常后，再将其集成到主Modbus轮询线程中
 *
 * 测试步骤：
 * 1. 首先尝试不同的从站地址（1, 3, 等）
 * 2. 尝试不同的波特率（2400, 4800）
 * 3. 尝试不同的寄存器起始地址
 * 4. 观察返回的原始数据格式
 *
 * @param parameter 线程参数(未使用)
 */
void stress_sensor_test_thread_entry(void *parameter)
{
    eMBMasterReqErrCode error_code = MB_MRE_NO_ERR;
    uint16_t reg_high, reg_low;
    uint32_t success_count = 0;
    uint32_t error_count = 0;

    rt_kprintf("\n========================================\n");
    rt_kprintf("  GMY400 Stress Sensor Test Started\n");
    rt_kprintf("========================================\n");
    rt_kprintf("  Slave Address: %d\n", STRESS_SLAVE_ADDR);
    rt_kprintf("  Register Start: 0x%04X\n", STRESS_REG_START);
    rt_kprintf("  Register Count: %d\n", STRESS_REG_NUM);
    rt_kprintf("  Baudrate: 9600 (当前系统波特率)\n");
    rt_kprintf("  注意: 传感器标称波特率为 2400/4800\n");
    rt_kprintf("       如果通信失败，需要修改系统波特率或传感器波特率\n");
    rt_kprintf("  Parity: None\n");
    rt_kprintf("  Range: 0~400 kN\n");
    rt_kprintf("========================================\n\n");

    /* 等待Modbus主站初始化完成 */
    rt_thread_mdelay(2000);

    while (1)
    {
        /* 读取应力传感器的保持寄存器 */
        error_code = eMBMasterReqReadHoldingRegister(
            STRESS_SLAVE_ADDR,
            STRESS_REG_START,
            STRESS_REG_NUM,
            RT_WAITING_FOREVER
        );

        if (error_code == MB_MRE_NO_ERR)
        {
            /* 从缓冲区读取寄存器值 (从站地址3对应缓冲区索引2) */
            reg_high = usMRegHoldBuf[STRESS_SLAVE_ADDR - 1][STRESS_REG_START];
            reg_low  = usMRegHoldBuf[STRESS_SLAVE_ADDR - 1][STRESS_REG_START + 1];

            g_stress_sensor_online = RT_TRUE;
            success_count++;

            /* 尝试解析数据 - 需要根据实际返回值调整 */
            /* 方案1: 假设为两个16位寄存器组成的32位整数或浮点数 */
            uint32_t combined = ((uint32_t)reg_high << 16) | reg_low;

            /* 尝试将原始值映射到0~400kN范围 */
            /* 这里需要根据实际测试数据调整换算公式 */
            float stress_kn = (combined / 65535.0f) * STRESS_RANGE_KN;

            g_stress_value_kn = stress_kn;

            /* 打印详细的调试数据 */
            int stress_int = (int)g_stress_value_kn;
            int stress_dec = (int)((g_stress_value_kn - stress_int) * 100);
            if (stress_dec < 0) stress_dec = -stress_dec;

            rt_kprintf("[%05d] Stress: %d.%02d kN (Raw: Reg[0]=0x%04X=%d, Reg[1]=0x%04X=%d, Combined=0x%08X=%u) | OK: %d, ERR: %d\n",
                       success_count + error_count,
                       stress_int, stress_dec,
                       reg_high, reg_high,
                       reg_low, reg_low,
                       combined, combined,
                       success_count, error_count);
        }
        else
        {
            /* 通信错误处理 */
            g_stress_sensor_online = RT_FALSE;
            error_count++;

            rt_kprintf("[%05d] Stress Error: Code=%d | OK: %d, ERR: %d\n",
                       success_count + error_count,
                       error_code,
                       success_count, error_count);

            /* 如果连续失败，给出提示 */
            if (error_count >= 5 && success_count == 0)
            {
                rt_kprintf("\n========================================\n");
                rt_kprintf("  通信失败可能原因:\n");
                rt_kprintf("  1. 从站地址不匹配 (当前: %d)\n", STRESS_SLAVE_ADDR);
                rt_kprintf("  2. 波特率不匹配 (系统: 9600, 传感器: 2400/4800)\n");
                rt_kprintf("  3. 寄存器地址不正确 (当前: 0x%04X)\n", STRESS_REG_START);
                rt_kprintf("  4. 接线问题 (检查485A/485B是否正确)\n");
                rt_kprintf("  5. 供电问题 (检查DC18V电源)\n");
                rt_kprintf("========================================\n\n");
            }
        }

        /* 每2秒读取一次 */
        rt_thread_mdelay(2000);
    }
}

/**
 * @brief 修改应力传感器的Modbus从站地址
 *
 * @param old_addr 当前从站地址
 * @param new_addr 新的从站地址（1-247）
 * @return RT_EOK 成功，-RT_ERROR 失败
 */
rt_err_t stress_change_slave_address(uint8_t old_addr, uint8_t new_addr)
{
    eMBMasterReqErrCode error_code;
    USHORT reg_value = (USHORT)new_addr;

    rt_kprintf("\n========================================\n");
    rt_kprintf("  Stress Sensor Address Change\n");
    rt_kprintf("========================================\n");
    rt_kprintf("  Old Address: %d (0x%02X)\n", old_addr, old_addr);
    rt_kprintf("  New Address: %d (0x%02X)\n", new_addr, new_addr);
    rt_kprintf("  Register: 0x%04X (假设)\n", STRESS_SLAVE_ADDR_REG);
    rt_kprintf("  注意: 寄存器地址需要根据实际手册确认\n");
    rt_kprintf("========================================\n\n");

    rt_kprintf("[Stress] Sending address change command...\n");

    /* 写从站地址配置寄存器 */
    error_code = eMBMasterReqWriteHoldingRegister(
        old_addr,                   // 使用旧地址发送命令
        STRESS_SLAVE_ADDR_REG,      // 从站地址配置寄存器
        reg_value,                  // 新地址值
        RT_WAITING_FOREVER          // 等待响应
    );

    if (error_code == MB_MRE_NO_ERR)
    {
        rt_kprintf("[Stress] ✓ Address changed successfully!\n");
        rt_kprintf("[Stress] ✓ New address: %d (0x%02X)\n", new_addr, new_addr);
        rt_kprintf("[Stress] ! Please verify with 'stress_test_addr %d'\n", new_addr);
        return RT_EOK;
    }
    else
    {
        rt_kprintf("[Stress] ✗ Failed to change address\n");
        rt_kprintf("[Stress] ✗ Error code: %d\n", error_code);
        rt_kprintf("[Stress] ! Check connections and current address\n");
        return -RT_ERROR;
    }
}

/**
 * @brief 使用指定地址测试读取应力值
 */
static void stress_test_address(uint8_t test_addr)
{
    eMBMasterReqErrCode error_code;
    uint16_t reg_high, reg_low;

    rt_kprintf("\n[Stress] Testing address %d...\n", test_addr);

    error_code = eMBMasterReqReadHoldingRegister(
        test_addr,              // 测试地址
        STRESS_REG_START,       // 应力值寄存器
        STRESS_REG_NUM,         // 读取2个寄存器
        RT_WAITING_FOREVER
    );

    if (error_code == MB_MRE_NO_ERR)
    {
        reg_high = usMRegHoldBuf[test_addr - 1][STRESS_REG_START];
        reg_low  = usMRegHoldBuf[test_addr - 1][STRESS_REG_START + 1];

        uint32_t combined = ((uint32_t)reg_high << 16) | reg_low;
        float stress = (combined / 65535.0f) * STRESS_RANGE_KN;

        rt_kprintf("[Stress] ✓ Address %d responds correctly!\n", test_addr);
        rt_kprintf("[Stress]   Raw: Reg[0]=0x%04X, Reg[1]=0x%04X\n", reg_high, reg_low);
        rt_kprintf("[Stress]   Combined: 0x%08X = %u\n", combined, combined);
        rt_kprintf("[Stress]   Stress: %.2f kN\n", stress);
    }
    else
    {
        rt_kprintf("[Stress] ✗ Address %d test failed\n", test_addr);
        rt_kprintf("[Stress] ✗ Error code: %d\n", error_code);
        rt_kprintf("[Stress] ! Device may not be at this address\n");
    }
}

/**
 * @brief MSH命令：修改应力传感器地址
 * 使用方法: stress_set_addr 1 3
 */
static void stress_set_addr(int argc, char **argv)
{
    uint8_t old_addr, new_addr;

    if (argc != 3)
    {
        rt_kprintf("\nUsage: stress_set_addr <old_addr> <new_addr>\n");
        rt_kprintf("Example: stress_set_addr 1 3\n");
        rt_kprintf("         (Change from address 1 to address 3)\n\n");
        return;
    }

    old_addr = atoi(argv[1]);
    new_addr = atoi(argv[2]);

    if (old_addr < 1 || old_addr > 247 || new_addr < 1 || new_addr > 247)
    {
        rt_kprintf("\nError: Address must be in range 1-247\n\n");
        return;
    }

    if (old_addr == new_addr)
    {
        rt_kprintf("\nError: Old and new addresses are the same!\n\n");
        return;
    }

    rt_kprintf("\n⚠️  WARNING: Ensure all other Modbus devices are disconnected!\n");
    rt_kprintf("⚠️  WARNING: Only the stress sensor should be on the bus!\n\n");

    stress_change_slave_address(old_addr, new_addr);
}
MSH_CMD_EXPORT(stress_set_addr, Change stress sensor slave address);

/**
 * @brief MSH命令：测试应力传感器地址
 * 使用方法: stress_test_addr 3
 */
static void stress_test_addr(int argc, char **argv)
{
    uint8_t test_addr;

    if (argc != 2)
    {
        rt_kprintf("\nUsage: stress_test_addr <address>\n");
        rt_kprintf("Example: stress_test_addr 3\n");
        rt_kprintf("         (Test if device responds at address 3)\n\n");
        return;
    }

    test_addr = atoi(argv[1]);

    if (test_addr < 1 || test_addr > 247)
    {
        rt_kprintf("\nError: Address must be in range 1-247\n\n");
        return;
    }

    stress_test_address(test_addr);
}
MSH_CMD_EXPORT(stress_test_addr, Test stress sensor address);

/**
 * @brief MSH命令：扫描Modbus总线上的所有设备
 * 使用方法: stress_scan_bus
 */
static void stress_scan_bus(int argc, char **argv)
{
    eMBMasterReqErrCode error_code;
    uint8_t found_count = 0;

    rt_kprintf("\n========================================\n");
    rt_kprintf("  Scanning Modbus Bus (Address 1-10)\n");
    rt_kprintf("========================================\n");
    rt_kprintf("  This may take ~30 seconds...\n\n");

    for (uint8_t addr = 1; addr <= 10; addr++)
    {
        rt_kprintf("[Scan] Testing address %d... ", addr);

        error_code = eMBMasterReqReadHoldingRegister(
            addr,
            0x0000,
            2,
            RT_WAITING_FOREVER
        );

        if (error_code == MB_MRE_NO_ERR)
        {
            rt_kprintf("✓ FOUND!\n");
            found_count++;
        }
        else
        {
            rt_kprintf("✗ No response\n");
        }

        rt_thread_mdelay(500);  /* 避免总线拥塞 */
    }

    rt_kprintf("\n========================================\n");
    rt_kprintf("  Scan Complete: %d device(s) found\n", found_count);
    rt_kprintf("========================================\n\n");
}
MSH_CMD_EXPORT(stress_scan_bus, Scan Modbus bus for devices);
