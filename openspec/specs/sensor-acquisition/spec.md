# Sensor Acquisition Capability

## Purpose
传感器数据采集模块负责从甲烷传感器和 ADC 多通道输入读取现场环境监测数据，并通过全局变量向通信和上报模块提供最新采样结果。
## Requirements
### Requirement: Methane Sensor Data Acquisition
系统 SHALL 通过 UART2 接口从甲烷传感器采集浓度数据。

#### Scenario: Valid frame received
- **WHEN** 接收到完整的 19 字节数据帧 (帧头 0xAC 0xAC)
- **AND** 校验和验证通过
- **THEN** 解析 PPM 值和 LEL% 值
- **AND** 更新全局变量 g_methane_ppm 和 g_methane_lel

#### Scenario: Invalid frame received
- **WHEN** 接收到的数据帧校验和错误
- **THEN** 丢弃该帧
- **AND** 等待下一个有效帧头

### Requirement: ADC Multi-Channel Acquisition
系统 SHALL 通过 ADC1 采集 5 个通道的模拟信号。

#### Scenario: Periodic ADC sampling
- **WHEN** ADC 采集线程每 500ms 触发
- **THEN** 依次读取 CH0/CH1/CH3/CH4/CH5 的 12 位原始值
- **AND** 更新全局变量 g_adc_ch0 ~ g_adc_ch5

#### Scenario: Fire sensor detection
- **WHEN** ADC CH5 (火焰传感器) 读取完成
- **THEN** 将原始值存储到 g_adc_ch5
- **AND** 可被上报模块读取用于火焰检测

### Requirement: Data Export via Global Variables
系统 SHALL 通过全局变量导出传感器数据供其他模块使用。

#### Scenario: Global variable access
- **WHEN** 任何模块需要读取传感器数据
- **THEN** 可直接访问 extern 声明的全局变量
- **AND** 无需调用函数接口

### Requirement: RDF-TC25 Stress Sensor Acquisition via Modbus RTU
系统 SHALL 通过 UART3 Modbus RTU 主站轮询地址 `3` 的 RDF-TC25/RDD-DH 应力传感器，并以牛顿为单位提供最新有效力值。

#### Scenario: Valid stress measurement
- **WHEN** Modbus 主轮询依次成功读取状态寄存器 `0x0008`、单位寄存器 `0x0068` 和力值寄存器 `0x0050-0x0051`
- **THEN** 根据状态寄存器的小数位解析有符号 32 位原始力值
- **AND** 根据仪表单位将测量值换算为 N
- **AND** 更新 `g_stress_value_n` 并将 `g_stress_sensor_online` 和 `g_stress_value_valid` 置为真

#### Scenario: Stress sensor communication failure
- **WHEN** 任一应力传感器 Modbus 读取失败
- **THEN** 将 `g_stress_sensor_online` 和 `g_stress_value_valid` 置为假
- **AND** 保留 `g_stress_value_n` 中最后一次有效力值

#### Scenario: Stress sensor data export
- **WHEN** 上报模块需要读取应力值
- **THEN** 可通过 `g_stress_value_n` 获取单位为 N 的最后一次有效测量值
- **AND** 可通过 `g_stress_sensor_online` 和 `g_stress_value_valid` 判断当前通信和数据状态

### Requirement: SHT30 Temperature/Humidity Sensor Acquisition via I2C1
系统 SHALL 通过软件 I2C1 总线 (PB6 SCL / PB7 SDA) 从 SHT30 传感器采集温度和湿度数据，并通过全局变量向其他模块提供最新读数。

#### Scenario: I2C1 bus initialization success
- **WHEN** 系统启动且 `sht30_init()` 被调用
- **AND** `RT_USING_I2C`、`RT_USING_I2C_BITOPS` 已在 `rtconfig.h` 中启用
- **AND** `drivers/board.h` 中 `BSP_USING_I2C1` 已定义且 SCL/SDA 引脚配置为 PB6/PB7
- **THEN** `rt_i2c_bus_device_find("i2c1")` 返回有效设备句柄
- **AND** 串口日志输出 "SHT30 initialized on i2c1"

#### Scenario: Periodic SHT30 single-shot read succeeds
- **WHEN** SHT30 采集线程按 `SHT30_READ_INTERVAL_MS` 周期触发
- **AND** I2C1 总线可用
- **THEN** 通过 `rt_i2c_transfer` 发送测量命令 `0x2C 0x06`
- **AND** 等待 ≥15ms 后读取 6 字节响应数据
- **AND** 对温度原始值和湿度原始值分别执行 CRC8 校验 (多项式 0x31, 初值 0xFF)
- **AND** 按公式 T = -45 + 175 × raw_t / 65535 计算摄氏温度
- **AND** 按公式 RH = 100 × raw_h / 65535 计算相对湿度
- **AND** 更新全局变量 `g_temperature_c` 和 `g_humidity_rh`

#### Scenario: I2C read failure — retain last valid value
- **WHEN** `rt_i2c_transfer` 返回值不是预期的 1 条已完成消息
- **OR** 读取的 6 字节数据中任一 CRC8 校验失败
- **THEN** 不更新 `g_temperature_c` 和 `g_humidity_rh`
- **AND** 保留上一次有效采样值
- **AND** 通过 `rt_kprintf` 输出错误日志（包含错误原因：I2C 错误码或 CRC 失败）

#### Scenario: Global variable export for other modules
- **WHEN** 任何模块（如上报线程）需要读取温湿度数据
- **THEN** 可直接通过 `extern float g_temperature_c;` 和 `extern float g_humidity_rh;` 访问最新值
- **AND** 无需调用额外函数接口

### Requirement: Oxygen Sensor Acquisition via ADC1 Channel 18 (PA4)
系统 SHALL 通过 RT-Thread ADC 设备框架从 ADC1 第 18 通道 (PA4) 采集氧气传感器模拟信号，并转换为氧浓度百分比通过全局变量提供。

#### Scenario: ADC device initialization success
- **WHEN** 系统启动且 `o2_sensor_init()` 被调用
- **AND** `RT_USING_ADC` 和 `BSP_USING_ADC1` 已在 `rtconfig.h` / `drivers/board.h` 中启用
- **THEN** `rt_device_find("adc1")` 返回有效设备句柄
- **AND** `rt_adc_enable()` 调用成功
- **AND** `drivers/board.c` 已将 PA4 配置为 ADC1_INP18 模拟输入
- **AND** 串口日志输出 "O2 sensor initialized on adc1 ch18"

#### Scenario: Periodic O2 ADC read succeeds
- **WHEN** O2 采集线程按 `O2_READ_INTERVAL_MS` 周期触发
- **AND** ADC1 设备可用
- **THEN** 通过 `rt_adc_read(dev, 18)` 读取通道 18 的 12 位原始值
- **AND** 按公式 `mV = raw × 3300 / 4096` 换算为毫伏
- **AND** 按公式 `O2(×10) = (mV - ZERO_OFFSET) × 209 / (FULL_SCALE - ZERO_OFFSET)` 计算氧浓度（×10）
- **AND** 若 O2(×10) 在 207–211 范围内，强制设为 209 (20.9% 空气稳定区)
- **AND** 更新全局变量 `g_o2_concentration = (float)O2(×10) / 10.0f`

#### Scenario: ADC initialization failure
- **WHEN** `rt_device_find("adc1")` 返回空设备句柄
- **OR** `rt_adc_enable()` 无法使能通道 18
- **THEN** `o2_sensor_init()` 返回错误
- **AND** 不创建 O2 采集线程

#### Scenario: Global variable export for other modules
- **WHEN** 任何模块（如上报线程）需要读取氧浓度数据
- **THEN** 可直接通过 `extern float g_o2_concentration;` 访问最新值
- **AND** 无需调用额外函数接口
