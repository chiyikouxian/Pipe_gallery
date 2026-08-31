/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-08-31     Administrator       Displacement sensor application
 */

#include "displacementSensorApp.h"
#include "freeModbusApp.h"

/* 全局变量定义 */
float g_displacement_value = 0.0f;
rt_bool_t g_displacement_sensor_online = RT_FALSE;

/**
 * @brief 打印位移传感器状态信息
 */
void displacement_print_status(void)
{
    if (g_displacement_sensor_online)
    {
        int disp_int = (int)g_displacement_value;
        int disp_dec = (int)((g_displacement_value - disp_int) * 1000);
        if (disp_dec < 0) disp_dec = -disp_dec;

        rt_kprintf("[Displacement] Online - Value: %d.%03d mm\n", disp_int, disp_dec);
    }
    else
    {
        rt_kprintf("[Displacement] Offline - No communication\n");
    }
}

/**
 * @brief 位移传感器测试线程入口
 *
 * 该线程用于独立测试位移传感器的Modbus通信
 * 在确认通信正常后，再将其集成到主Modbus轮询线程中
 *
 * @param parameter 线程参数(未使用)
 */
void displacement_sensor_test_thread_entry(void *parameter)
{
    eMBMasterReqErrCode error_code = MB_MRE_NO_ERR;
    uint16_t reg_low;
    uint32_t success_count = 0;
    uint32_t error_count = 0;

    rt_kprintf("\n========================================\n");
    rt_kprintf("  Displacement Sensor Test Started\n");
    rt_kprintf("========================================\n");
    rt_kprintf("  Slave Address: %d\n", DISPLACEMENT_SLAVE_ADDR);
    rt_kprintf("  Register Start: 0x%04X\n", DISPLACEMENT_REG_START);
    rt_kprintf("  Register Count: %d\n", DISPLACEMENT_REG_NUM);
    rt_kprintf("  Baudrate: 9600\n");
    rt_kprintf("  Parity: None\n");
    rt_kprintf("  Scale Factor: %.3f (1 unit = 0.025mm)\n", DISPLACEMENT_SCALE_FACTOR);
    rt_kprintf("========================================\n\n");

    /* 等待Modbus主站初始化完成 */
    rt_thread_mdelay(2000);

    while (1)
    {
        /* 读取位移传感器的保持寄存器 */
        error_code = eMBMasterReqReadHoldingRegister(
            DISPLACEMENT_SLAVE_ADDR,
            DISPLACEMENT_REG_START,
            DISPLACEMENT_REG_NUM,
            RT_WAITING_FOREVER
        );

        if (error_code == MB_MRE_NO_ERR)
        {
            /* 从缓冲区读取寄存器值 (从站地址1对应缓冲区索引0) */
            reg_low  = usMRegHoldBuf[DISPLACEMENT_SLAVE_ADDR - 1][DISPLACEMENT_REG_START + 1];

            g_displacement_sensor_online = RT_TRUE;
            success_count++;

            /* 位移传感器数据只在 Reg[1] 中 */
            uint16_t raw_value = reg_low;

            /* 减去零点偏移，然后转换为mm */
            int32_t adjusted_raw = (int32_t)raw_value - DISPLACEMENT_ZERO_OFFSET;
            if (adjusted_raw < 0) adjusted_raw = 0;  /* 防止负值 */

            g_displacement_value = adjusted_raw * DISPLACEMENT_SCALE_FACTOR;

            /* 打印简洁的数据 */
            int disp_int = (int)g_displacement_value;
            int disp_dec = (int)((g_displacement_value - disp_int) * 1000);
            if (disp_dec < 0) disp_dec = -disp_dec;

            rt_kprintf("[%05d] Displacement: %d.%03d mm (Raw: %d) | OK: %d, ERR: %d\n",
                       success_count + error_count,
                       disp_int, disp_dec,
                       raw_value,
                       success_count, error_count);
        }
        else
        {
            /* 通信错误处理 */
            g_displacement_sensor_online = RT_FALSE;
            error_count++;

            rt_kprintf("[%05d] Displacement Error: Code=%d | OK: %d, ERR: %d\n",
                       success_count + error_count,
                       error_code,
                       success_count, error_count);
        }

        /* 每1秒读取一次 */
        rt_thread_mdelay(1000);
    }
}

/**
 * @brief 修改位移传感器的Modbus从站地址
 *
 * @param old_addr 当前从站地址
 * @param new_addr 新的从站地址（1-247）
 * @return RT_EOK 成功，-RT_ERROR 失败
 */
rt_err_t displacement_change_slave_address(uint8_t old_addr, uint8_t new_addr)
{
    eMBMasterReqErrCode error_code;
    USHORT reg_value = (USHORT)new_addr;

    rt_kprintf("\n========================================\n");
    rt_kprintf("  Displacement Sensor Address Change\n");
    rt_kprintf("========================================\n");
    rt_kprintf("  Old Address: %d (0x%02X)\n", old_addr, old_addr);
    rt_kprintf("  New Address: %d (0x%02X)\n", new_addr, new_addr);
    rt_kprintf("  Register: 0x0004\n");
    rt_kprintf("  Baudrate: 9600, 8N1\n");
    rt_kprintf("  Modbus Frame: %02X 06 00 04 00 %02X [CRC]\n", old_addr, new_addr);
    rt_kprintf("========================================\n\n");

    rt_kprintf("[DISP] Sending address change command...\n");

    /* 写从站地址配置寄存器 (0x0004) */
    error_code = eMBMasterReqWriteHoldingRegister(
        old_addr,           // 使用旧地址发送命令
        0x0004,             // 从站地址配置寄存器
        reg_value,          // 新地址值
        RT_WAITING_FOREVER  // 等待响应
    );

    if (error_code == MB_MRE_NO_ERR)
    {
        rt_kprintf("[DISP] ✓ Address changed successfully!\n");
        rt_kprintf("[DISP] ✓ New address: %d (0x%02X)\n", new_addr, new_addr);
        rt_kprintf("[DISP] ✓ Change is permanent (saved in non-volatile memory)\n");
        rt_kprintf("[DISP] ! Please run 'disp_test_addr %d' to verify\n", new_addr);
        return RT_EOK;
    }
    else
    {
        rt_kprintf("[DISP] ✗ Failed to change address\n");
        rt_kprintf("[DISP] ✗ Error code: %d\n", error_code);
        rt_kprintf("[DISP] ! Check connections and ensure no other device uses address %d\n", old_addr);
        return -RT_ERROR;
    }
}

/**
 * @brief 使用指定地址测试读取位移值
 */
static void displacement_test_address(uint8_t test_addr)
{
    eMBMasterReqErrCode error_code;
    uint16_t raw_value;

    rt_kprintf("\n[DISP] Testing address %d...\n", test_addr);

    error_code = eMBMasterReqReadHoldingRegister(
        test_addr,          // 测试地址
        0x0000,             // 位移值寄存器
        2,                  // 读取2个寄存器
        RT_WAITING_FOREVER
    );

    if (error_code == MB_MRE_NO_ERR)
    {
        raw_value = usMRegHoldBuf[test_addr - 1][1];
        int32_t adjusted = (int32_t)raw_value - 74;
        if (adjusted < 0) adjusted = 0;
        float displacement = adjusted * 0.025f;

        rt_kprintf("[DISP] ✓ Address %d responds correctly!\n", test_addr);
        rt_kprintf("[DISP]   Raw value: %d\n", raw_value);
        rt_kprintf("[DISP]   Displacement: %.2f mm\n", displacement);
    }
    else
    {
        rt_kprintf("[DISP] ✗ Address %d test failed\n", test_addr);
        rt_kprintf("[DISP] ✗ Error code: %d\n", error_code);
        rt_kprintf("[DISP] ! Device may not be at this address\n");
    }
}

/**
 * @brief MSH命令：修改位移传感器地址
 * 使用方法: disp_set_addr 1 2
 */
static void disp_set_addr(int argc, char **argv)
{
    uint8_t old_addr, new_addr;

    if (argc != 3)
    {
        rt_kprintf("\nUsage: disp_set_addr <old_addr> <new_addr>\n");
        rt_kprintf("Example: disp_set_addr 1 2\n");
        rt_kprintf("         (Change from address 1 to address 2)\n\n");
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
    rt_kprintf("⚠️  WARNING: Only the displacement sensor should be on the bus!\n\n");

    displacement_change_slave_address(old_addr, new_addr);
}
MSH_CMD_EXPORT(disp_set_addr, Change displacement sensor slave address);

/**
 * @brief MSH命令：测试位移传感器地址
 * 使用方法: disp_test_addr 2
 */
static void disp_test_addr(int argc, char **argv)
{
    uint8_t test_addr;

    if (argc != 2)
    {
        rt_kprintf("\nUsage: disp_test_addr <address>\n");
        rt_kprintf("Example: disp_test_addr 2\n");
        rt_kprintf("         (Test if device responds at address 2)\n\n");
        return;
    }

    test_addr = atoi(argv[1]);

    if (test_addr < 1 || test_addr > 247)
    {
        rt_kprintf("\nError: Address must be in range 1-247\n\n");
        return;
    }

    displacement_test_address(test_addr);
}
MSH_CMD_EXPORT(disp_test_addr, Test displacement sensor address);
