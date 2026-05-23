## ADDED Requirements

### Requirement: SHT30 Temperature/Humidity Sensor Acquisition via I2C1
系统 SHALL 通过软件 I2C1 总线 (PB6 SCL / PB7 SDA) 从 SHT30 传感器采集温度和湿度数据，并通过全局变量向其他模块提供最新读数。

#### Scenario: I2C1 bus initialization success
- **WHEN** 系统启动且 `sht30_init()` 被调用
- **AND** `RT_USING_I2C`、`RT_USING_I2C_BITOPS`、`RT_USING_I2C1` 已在 rtconfig.h 中启用
- **AND** `drivers/board.h` 中 `BSP_USING_I2C1` 已定义且 SCK/SDA 引脚配置为 PB6/PB7
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
- **WHEN** `rt_i2c_transfer` 返回非零错误码
- **OR** 读取的 6 字节数据中任一 CRC8 校验失败
- **THEN** 不更新 `g_temperature_c` 和 `g_humidity_rh`
- **AND** 保留上一次有效采样值
- **AND** 通过 `rt_kprintf` 输出错误日志（包含错误原因：I2C 错误码或 CRC 失败）

#### Scenario: Global variable export for other modules
- **WHEN** 任何模块（如上报线程）需要读取温湿度数据
- **THEN** 可直接通过 `extern float g_temperature_c;` 和 `extern float g_humidity_rh;` 访问最新值
- **AND** 无需调用额外函数接口
