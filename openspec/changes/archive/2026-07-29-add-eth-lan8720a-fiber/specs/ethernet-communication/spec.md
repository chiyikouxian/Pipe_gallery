## ADDED Requirements

### Requirement: Ethernet MAC Driver with LAN8720A PHY
系统 SHALL 通过 STM32H743 内置 ETH MAC 以 RMII 模式驱动外部 LAN8720A 以太网 PHY。

#### Scenario: EMAC device initialization
- **WHEN** 系统启动时执行 `INIT_DEVICE_EXPORT(rt_hw_stm32_eth_init)`
- **THEN** ETH MAC 硬件初始化完成 (HAL_ETH_Init)
- **AND** 通过 MDIO 自动扫描发现 LAN8720A PHY 地址
- **AND** PHY 软件复位成功 (BCR bit 15)
- **AND** 启动自动协商并强制 100M 全双工

#### Scenario: PHY link up detection
- **WHEN** 以太网电缆连接并自动协商完成
- **THEN** PHY 链路状态通过轮询定时器或中断检测
- **AND** `eth_device_linkchange(RT_TRUE)` 通知上层协议栈

#### Scenario: PHY link down detection
- **WHEN** 以太网电缆断开
- **THEN** `eth_device_linkchange(RT_FALSE)` 通知上层
- **AND** LWIP 协议栈停止发送数据直到链路恢复

#### Scenario: Missing hardware reset pin
- **WHEN** LAN8720A 模块未引出 nRST 硬件复位引脚
- **THEN** 驱动通过 MDIO 写入 PHY 寄存器完成软件复位
- **AND** 不使用/绑定 硬件复位 GPIO 引脚避免系统错误

### Requirement: LWIP TCP/IP Stack Over Ethernet
系统 SHALL 通过 LWIP 协议栈在以太网接口上实现 TCP/IP 网络通信。

#### Scenario: Static IP assignment
- **WHEN** 以太网接口初始化后
- **THEN** 使用静态 IPv4 地址 `192.168.1.30/24`
- **AND** 网关设置为 `192.168.1.1`

#### Scenario: TCP client transmission
- **WHEN** 应用层通过 socket API 发送数据
- **THEN** LWIP 通过 `ethernetif` 接口将数据帧交给 ETH MAC 驱动发送

#### Scenario: Incoming packet reception
- **WHEN** ETH MAC 接收到以太网帧
- **THEN** 通过 `eth_device_ready()` 通知 ethernetif 层
- **AND** LWIP 协议栈处理并将数据传递给上层应用 socket

### Requirement: Ethernet TCP JSON Data Reporting
系统 SHALL 通过以太网 TCP 连接定时上报传感器数据 JSON 给 PC 服务器。

#### Scenario: Periodic data report
- **WHEN** 以太网链路建立且 TCP 连接到 `192.168.1.100:8080`
- **THEN** 每 `ETH_REPORT_INTERVAL` (3000ms) 发送一个包含所有传感器数据的 JSON 字符串
- **AND** JSON 包含: `device_id`, ADC 通道 (ch0-ch5), 甲烷 (ppm, lel), 电压 (a/b/c), 电流 (a/b/c), 流量, 温度, 湿度, 氧气浓度, 火焰状态

#### Scenario: TCP connection failure and recovery
- **WHEN** TCP send 连续失败 3 次
- **THEN** 关闭当前 socket
- **AND** 等待 `ETH_RECONNECT_DELAY` (5000ms) 后重新连接

#### Scenario: Ethernet unavailable fallback
- **WHEN** 以太网链路从未建立
- **THEN** TCP 客户端线程持续尝试连接
- **AND** 不影响其他通信通道 (LoRa, MQTT/WiFi) 正常工作
