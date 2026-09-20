# Pipe Gallery Node

基于 `STM32H743XI + RT-Thread` 的管廊监测节点项目。

本项目当前已经完成多传感器采集、多链路上报，以及 `LAN8720A` 以太网到 PC 的直连/光纤收发器链路验证。

## 1. 项目概况

硬件与软件基础：
- MCU：`STM32H743XI`
- RTOS：`RT-Thread v4.0.3`
- 语言：`C`
- 工程方式：`RT-Thread Studio / SCons / Debug make`

项目目标：
- 采集甲烷、火焰、氧气、温湿度、三相电压/电流、水流量、位移和应力等数据
- 通过多种链路上报或转发数据
- 当前重点已完成 LAN8720A 有线以太网链路验证

## 2. 当前进度

当前阶段状态：`以太网链路阶段已完成`

已完成的关键里程碑：
- LAN8720A PHY 初始化成功
- RMII 链路稳定工作在 `100Mbps / full-duplex`
- MCU 静态 IP 工作正常：`192.168.1.30`
- PC 静态 IP 工作正常：`192.168.1.100`
- `MCU -> 网线 -> PC` 直连链路验证成功
- `MCU -> 网线 -> 光纤收发器A -> 光纤 -> 光纤收发器B -> 网线 -> PC` 链路验证成功
- 双向 `ping` 验证成功
- TCP 连接 `192.168.1.100:8080` 验证成功
- PC 成功收到 MCU 周期性发送的 JSON 数据

本阶段在代码中的关键修复：
- 关闭 `RT_LWIP_DHCP`，改为固定 IP 调试
- 使能 `ETH_IRQn`，修复“能发不能收”的 RX 中断问题
- LAN8720A 无硬件 `nRST` 时改为软件复位

## 3. 已完成功能

### 3.1 采集类功能

- 甲烷传感器采集
- 五路火焰传感器 ADC 采集
- 氧气传感器 ADC 采集
- SHT30 温湿度采集
- Modbus 电压/电流/流量/位移采集
- RDF-TC25/RDD-DH 应力传感器采集（从站地址 `3`，单位 `N`）

### 3.2 通信与上报功能

- UART 控制台调试
- Modbus RTU 主站轮询
- ESP8266 + 华为云 IoTDA MQTT 上报
- UART5 LoRa 上报
- 以太网 TCP 客户端上报

### 3.3 已验证的以太网行为

- PHY 地址检测到 `0x01`
- 链路状态稳定：`link up / 100Mbps / full-duplex`
- MCU 能 `ping 192.168.1.100`
- PC 能 `ping 192.168.1.30`
- MCU 串口出现：
  - `connected to 192.168.1.100:8080`
  - `Sent N bytes via ETH`
- PC 监听服务可以收到来自 `192.168.1.30` 的 JSON

## 4. 当前数据流

当前以太网上报的是 TCP 负载中的 JSON 字符串，主要字段包括：
- `device_id`
- `ch0 ch1 ch3 ch4 ch5`
- `methane_ppm`
- `methane_lel`
- `voltage_a voltage_b voltage_c`
- `current_a current_b current_c`
- `flow`
- `temperature`
- `humidity`
- `o2`
- `displacement`
- `stress`（单位：`N`，保留两位小数）
- `flame`

典型数据格式示例：

```json
{"device_id":"pipe_gallery_node_01","data":{"ch0":3339,"ch1":7680,"ch3":1867,"ch4":3655,"ch5":4607,"methane_ppm":0,"methane_lel":0,"voltage_a":0.00,"voltage_b":0.00,"voltage_c":0.00,"current_a":0.00,"current_b":0.00,"current_c":0.00,"flow":0.00,"temperature":0.00,"humidity":0.00,"o2":33.90,"displacement":0.00,"stress":0.33,"flame":0}}
```

## 5. 系统线程与运行模块

从 `applications/main.c` 当前启用情况看，系统主要运行：
- `uart4_rx`：ESP8266 接收线程
- `huawei_cloud_init()`：华为云上报线程
- `md_m_poll`：Modbus 主站轮询线程
- `md_m_send`：Modbus 数据读取线程
- `line_sensor_init()`：火焰传感器采集
- `sht30_init()`：温湿度采集
- `o2_sensor_init()`：氧气采集
- `eth_app_init()`：以太网 TCP 客户端线程
- `lora_tx`：LoRa 发送线程
- `uart2_rx`：甲烷传感器接收线程

## 6. 引脚分配

### 6.1 UART 分配

| 外设 | 引脚 | 用途 | 说明 |
|------|------|------|------|
| UART1 | `PA9/PA10` | 控制台 | 串口终端 |
| UART2 | `PD5/PD6` | 甲烷传感器 | 原来为 `PA2/PA3`，已因以太网复用调整 |
| UART3 | `PB10/PB11` | Modbus RTU | 水表/位移传感器/应力传感器/三相电表 |
| UART4 | `PA12/PA11` | ESP8266 | 华为云 MQTT（TX/RX） |
| UART5 | `PC12/PD2` | LoRa | ATK-LORA-01 |

### 6.2 Modbus 控制

| 信号 | 引脚 | 说明 |
|------|------|------|
| RS485 方向控制 | `PA15` | Modbus 收发方向 |

### 6.3 I2C 分配

| 外设 | 引脚 | 用途 |
|------|------|------|
| I2C1 | `PB6/PB7` | SHT30 温湿度 |

### 6.4 ADC 分配

当前实际 ADC 复用关系以 `drivers/board.c` 为准：

| 通道/用途 | 引脚 | ADC 通道 | 说明 |
|----------|------|----------|------|
| Flame A1 | `PA0` | `ADC1_INP0` | 使用模拟开关 |
| Flame A2 | `PA3` | `ADC1_INP15` | 从 `PA1` 迁移 |
| Flame A3 | `PA6` | `ADC1_INP3` | |
| Flame A4 | `PA5` | `ADC1_INP19` | 从 `PC4` 迁移 |
| Flame A5 | `PB1` | `ADC1_INP5` | |
| O2 Sensor | `PA4` | `ADC1_INP18` | 从 `PC5` 迁移；核心板 J1-35 引出 |

### 6.5 Ethernet RMII 分配

当前以太网使用 `LAN8720A + RMII`：

| STM32 引脚 | RMII 信号 | 说明 |
|-----------|-----------|------|
| `PA1` | `REF_CLK` | 50MHz 参考时钟 |
| `PA2` | `MDIO` | 管理数据线 |
| `PA7` | `CRS_DV` | 载波/数据有效 |
| `PC1` | `MDC` | 管理时钟 |
| `PC4` | `RXD0` | 以太网接收 0 |
| `PC5` | `RXD1` | 以太网接收 1 |
| `PG11` | `TX_EN` | 发送使能 |
| `PG13` | `TXD0` | 以太网发送 0 |
| `PG14` | `TXD1` | 以太网发送 1 |
| `PD3` | 预留 | 当前 LAN8720A 模块无 `nRST`，未实际用于硬复位 |

## 7. 网络配置

### 7.1 MCU 侧

- IP：`192.168.1.30`
- Netmask：`255.255.255.0`
- Gateway：`192.168.1.1`
- 工作模式：静态 IP
- 协议：TCP Client

### 7.2 PC 侧

- IP：`192.168.1.100`
- Netmask：`255.255.255.0`
- 测试端口：`8080`

### 7.3 以太网测试拓扑

直连铜缆：

`MCU -> RJ45 网线 -> PC`

光纤链路：

`MCU -> RJ45 网线 -> 光纤收发器A -> 光纤 -> 光纤收发器B -> RJ45 网线 -> PC`

## 8. 测试方法

### 8.1 PC 侧监听服务

```powershell
python tcp_server.py
```

### 8.2 PC 侧检查监听

```cmd
netstat -ano | findstr 8080
```

预期包含：

```text
0.0.0.0:8080   LISTENING
```

### 8.3 双向 ping

PC 测 MCU：

```cmd
ping 192.168.1.30
```

MCU 测 PC：

```sh
ping 192.168.1.100
```

### 8.4 成功判据

- PC 能 ping 通 MCU
- MCU 能 ping 通 PC
- MCU 串口出现 TCP 连接成功日志
- PC 收到周期性 JSON

## 9. 本阶段关键经验

- `RT_LWIP_DHCP` 不适合当前直连 PC 调试场景，必须关闭
- `ETH_IRQn` 不开时会出现：
  - PC 能学到 MCU 的 ARP
  - MCU 看起来链路已建立
  - 但 ping 回复和 TCP 建连失败
- 当前 LAN8720A 模块无 `nRST`，软件复位即可正常工作
- 光纤收发器 `LINK/ACT` 闪烁通常表示链路正在收发数据，属正常现象

## 10. 关键文件

- `applications/main.c`
- `applications/Apps/ethApp.c`
- `applications/Apps/MethaneSensorApp.c`
- `applications/Apps/huaweiCloudApp.c`
- `applications/Apps/loraApp.c`
- `applications/Apps/sht30App.c`
- `applications/Apps/o2SensorApp.c`
- `drivers/board.c`
- `drivers/board.h`
- `drivers/drv_eth.c`
- `drivers/include/drv_eth.h`
- `tcp_server.py`
- `openspec/changes/add-eth-lan8720a-fiber/`

## 11. 构建说明

常用方式：
- RT-Thread Studio 内部构建
- `Debug/` 目录下 `make`
- `scons` / RT-Thread 相关构建流程

当前项目的硬件联调结论以真实板级测试为准。
