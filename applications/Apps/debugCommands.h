/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-09-18     ideapad15s   Debug MSH commands for sensor testing
 */

#ifndef APPLICATIONS_APPS_DEBUGCOMMANDS_H_
#define APPLICATIONS_APPS_DEBUGCOMMANDS_H_

/* MSH命令已通过MSH_CMD_EXPORT导出，无需额外函数声明 */

/*
 * 可用的MSH调试命令：
 *
 * sensors          - 显示所有传感器数据汇总
 * flame            - 显示火焰传感器详细状态
 * env              - 显示环境传感器（温湿度、氧气、甲烷）
 * modbus           - 显示Modbus设备状态
 * monitor [sec]    - 实时监控传感器数据（可选刷新间隔秒数）
 * test <sensor>    - 测试单个传感器
 *
 * 应力传感器专用命令（在stressSensorApp.c中定义）：
 * stress_scan_bus           - 扫描Modbus总线设备
 * stress_test_addr <addr>   - 测试指定从站地址
 * stress_set_addr <old> <new> - 修改从站地址
 */

#endif /* APPLICATIONS_APPS_DEBUGCOMMANDS_H_ */
