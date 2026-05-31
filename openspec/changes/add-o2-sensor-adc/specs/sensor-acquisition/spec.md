## ADDED Requirements

### Requirement: Oxygen Sensor Acquisition via ADC1 Channel 8 (PC5)
系统 SHALL 通过 RT-Thread ADC 设备框架从 ADC1 第 8 通道 (PC5) 采集氧气传感器模拟信号，并转换为氧浓度百分比通过全局变量提供。

#### Scenario: ADC device initialization success
- **WHEN** 系统启动且 `o2_sensor_init()` 被调用
- **AND** `RT_USING_ADC` 和 `BSP_USING_ADC1` 已在 `rtconfig.h` / `drivers/board.h` 中启用
- **THEN** `rt_device_find("adc1")` 返回有效设备句柄
- **AND** `rt_adc_enable()` 调用成功
- **AND** 串口日志输出 "O2 sensor initialized on adc1 ch8"

#### Scenario: Periodic O2 ADC read succeeds
- **WHEN** O2 采集线程按 `O2_READ_INTERVAL_MS` 周期触发
- **AND** ADC1 设备可用
- **THEN** 通过 `rt_adc_read(dev, 8, &raw)` 读取通道 8 的 12 位原始值
- **AND** 按公式 `mV = raw × 3300 / 4096` 换算为毫伏
- **AND** 按公式 `O2(×10) = (mV - ZERO_OFFSET) × 209 / (FULL_SCALE - ZERO_OFFSET)` 计算氧浓度（×10）
- **AND** 若 O2(×10) 在 207–211 范围内，强制设为 209 (20.9% 空气稳定区)
- **AND** 更新全局变量 `g_o2_concentration = (float)O2(×10) / 10.0f`

#### Scenario: ADC read failure — retain last valid value
- **WHEN** `rt_adc_read()` 返回非零错误码
- **THEN** 不更新 `g_o2_concentration`
- **AND** 保留上一次有效采样值
- **AND** 通过 `rt_kprintf` 输出错误日志

#### Scenario: Global variable export for other modules
- **WHEN** 任何模块（如上报线程）需要读取氧浓度数据
- **THEN** 可直接通过 `extern float g_o2_concentration;` 访问最新值
- **AND** 无需调用额外函数接口
