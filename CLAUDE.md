# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在本仓库中处理代码时提供指导。

## 项目概述

这是一个基于 **STM32H743XI** (ARM Cortex-M7) 的 **RT-Thread RTOS** 嵌入式项目，用于管廊监测节点系统。系统集成了传感器数据采集（甲烷、火焰、电流、流量）、Modbus RTU 工业通信以及通过 WiFi/MQTT 实现的物联网连接。

**核心硬件：** STM32H743XI 微控制器
**实时操作系统：** RT-Thread v4.0.3
**构建系统：** SCons

## 构建命令

### 编译项目
```bash
# 编译项目（生成 rt-thread.elf）
scons

# 清理构建产物
scons -c

# 完全重新编译
scons -c && scons
```

### 前置条件
- ARM GCC 工具链必须在 PATH 中或通过 RTT_EXEC_PATH 环境变量设置
- 工具链前缀：`arm-none-eabi-`
- 已安装 Python 和 SCons

### 输出文件
- 二进制文件：`rt-thread.elf`（主目标）
- 链接脚本：`linkscripts/STM32H743XIHx/link.lds`

## 代码架构

### 三层架构

```
applications/              # 应用层 - 业务逻辑
├── Apps/                  # 应用模块
│   ├── NodeApp.c/h        # 节点数据结构（2个节点）
│   ├── freeModbusApp.c/h  # Modbus 主站通信
│   ├── uartApp.c/h        # UART4/UART5 通信
│   ├── MethaneSensorApp.c/h  # 甲烷传感器采集
│   ├── fireSensorApp.c/h  # 火焰传感器检测
│   └── wifiApp.c/h        # WiFi/MQTT 物联网连接
├── heads.h                # 统一头文件包含
└── main.c                 # 入口点和线程创建

drivers/                   # 硬件抽象层和板级支持包
├── drv_usart.c            # UART 驱动
├── drv_adc.c              # ADC 驱动
├── drv_gpio.c             # GPIO 驱动
└── board.c/h              # 板级配置

rt-thread/                 # RT-Thread RTOS 内核和组件
├── src/                   # 内核源码
├── components/            # 组件（驱动、网络、finsh）
└── libcpu/                # CPU 移植层
```

### 活动线程 (main.c)

系统启动时创建以下线程：

1. **md_m_poll**（优先级 10）：Modbus 主站轮询循环
2. **md_m_send**（优先级 30）：Modbus 数据发送/从从站设备接收
3. **uart5_thread**（优先级 25）：UART5 串口通信
4. **get_methane_thread**（优先级 25）：甲烷传感器数据采集

**注意：** main.c 中有几个线程被注释掉了（uart4_thread、wifi_thread），目前未激活。

### 节点数据结构

系统管理多个监测节点的数据（NODENUM = 2）：

```c
typedef struct Node {
    float CH1_A[3];   // 三相电流 [AC相, BC相, CA相]
    float Flow;       // 水流量
    char Flame;       // 火焰传感器（1=有火, 0=无火）
    float Methane;    // 甲烷浓度
} Node;
```

访问方式：`extern Node node[NODENUM];`

## Modbus 通信

### Modbus 配置 (freeModbusApp.c/h)

- **协议：** Modbus RTU 主站模式
- **端口：** PORT_NUM = 2 (UART2)
- **波特率：** 9600
- **校验：** 无校验 (MB_PAR_NONE)
- **轮询周期：** 500ms

### 从站设备

**电表（电流表）**
- 从站地址：2
- 起始寄存器：0x10E
- 寄存器数量：6（3对用于三相电流）
- 数据格式：32位浮点数（大端序）

**水表**
- 从站地址：1
- 起始寄存器：1
- 寄存器数量：2（流量）
- 数据格式：32位浮点数（大端序）

### Modbus 数据流

```
从站设备 → UART (RTU) → FreeModbus 协议栈 → usMRegHoldBuf[][]
→ 转换为浮点数 → 更新 node[].CH1_A[] / node[].Flow
```

**重要：** 数据首先存储在 `usMRegHoldBuf[从站地址-1][寄存器]` 数组中，然后转换为浮点数并赋值给节点结构体。

## UART 配置

### UART5
- **缓冲区大小：** 128 字节
- **线程：** uart5_thread
- **优先级：** 25
- **状态：** 启用

### UART4
- **缓冲区大小：** 256 字节
- **线程：** uart4_thread（在 main.c 中被注释）
- **状态：** 禁用（当前未启用）

### 控制台
- **设备：** uart1 (RT_CONSOLE_DEVICE_NAME)
- **缓冲区大小：** 256 字节

## RT-Thread 配置

### 关键 rtconfig.h 设置

- **时钟节拍：** 1000 Hz (RT_TICK_PER_SECOND)
- **最大线程优先级：** 32
- **主线程栈大小：** 4096 字节
- **主线程优先级：** 10
- **Finsh shell：** 启用（MSH 模式）
- **IPC：** 信号量、互斥锁、事件、邮箱、消息队列全部启用
- **内存：** 小内存算法

### RT-Thread 重要原则

1. **线程优先级：** 数值越小优先级越高（0 为最高优先级）
2. **线程创建：** 使用 `rt_thread_create()` 然后 `rt_thread_startup()`
3. **延时：** 使用 `rt_thread_mdelay(ms)` 而不是原始 HAL 延时
4. **日志输出：** 使用 `rt_kprintf()` 进行调试输出
5. **设备访问：** 使用 RT-Thread 设备框架 (rt_device_find/open/read/write)

## WiFi/MQTT 配置（当前禁用）

WiFi/MQTT 功能已实现但在 main.c 中被注释掉：

- **SSID：** "Win10-2018PYBHT 2565"
- **密码：** "1&H09h78"
- **MQTT 代理：** 华为云物联网平台
  - 主机：48931f72a4.st1.iotda-device.cn-north-4.myhuaweicloud.com
  - 端口：1883
  - 客户端 ID：67bedcb724d772325520a4c7_pipe_1_0_0_2025022804
- **数据格式：** JSON
- **主题格式：** $oc/devices/{device_id}/sys/properties/report

## FreeModbus 软件包

位于 `packages/freemodbus-latest/`：
- **核心：** modbus/ 目录包含协议实现
- **移植：** port/ 目录包含 RT-Thread 特定适配
- **缓冲区：** `usMRegHoldBuf[][]` 和 `usMRegInBuf[][]` 存储寄存器数据

## 常用开发模式

### 添加新传感器

1. 在 `applications/Apps/` 中创建传感器应用文件
2. 在 NodeApp.h 的 Node 结构体中添加传感器数据字段
3. 在传感器应用中创建采集线程
4. 在 main.c 中注册线程创建
5. 更新 heads.h 包含新的头文件

### 修改 Modbus 从站

1. 在 freeModbusApp.h 中更新从站地址定义
2. 修改寄存器起始地址/数量定义
3. 在 freeModbusApp.c 的 send_thread_entry() 中更新读取新寄存器
4. 将寄存器数据转换为适当格式
5. 赋值给 node[] 结构体

### 线程同步

使用 RT-Thread IPC 机制：
- 信号量用于信号传递
- 互斥锁用于保护共享数据（如 node[] 数组）
- 消息队列用于线程间通信

## 重要注意事项

1. **字符编码：** 源文件中的部分注释使用中文字符（GB2312/GBK 编码）
2. **初始化保护：** main.c 使用 `is_init` 标志防止重复初始化
3. **错误处理：** Modbus 操作返回 eMBMasterReqErrCode；检查 MB_MRE_NO_ERR
4. **字节序：** Modbus 数据为大端序 (data_H << 16 | data_L)
5. **线程清理：** 出错时，线程通过 __exit 段中的 rt_thread_delete() 删除

## 硬件配置

硬件外设通过 STM32CubeMX 配置：
- **配置文件：** cubemx/cubemx.ioc
- **HAL 库：** libraries/STM32H7xx_HAL_Driver/
- **生成代码：** cubemx/Src/ 和 cubemx/Inc/

修改 .ioc 文件后，重新生成代码并确保驱动集成保持完整。