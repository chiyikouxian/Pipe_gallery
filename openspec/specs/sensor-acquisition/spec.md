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
