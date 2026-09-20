# 管廊监测节点传感器逐个调试指南

## 📌 调试顺序

按以下顺序依次安装和调试传感器：

1. 火焰传感器（五通道）
2. 温湿度传感器（SHT30）
3. 甲烷气体传感器
4. 氧气传感器（O2）
5. 位移传感器
6. 应力传感器（GMY400）
7. 电表（三相电流表）

⚠️ **注意：水表不调试（仅作为参考）**

---

## 🔧 通用调试步骤

每个传感器的调试流程：

1. ✅ **硬件检查** - 确认接线、供电、引脚配置
2. ✅ **编译烧录** - 编译项目并烧录到STM32
3. ✅ **串口监控** - 连接串口终端查看日志输出
4. ✅ **功能验证** - 验证传感器数据读取是否正常
5. ✅ **数据校验** - 对比实际值与传感器读数
6. ✅ **稳定性测试** - 长时间运行验证稳定性

---

## 1️⃣ 火焰传感器（五通道模拟输出）

### 硬件接口

| 通道 | MCU引脚 | ADC通道 | 说明 |
|-----|--------|---------|-----|
| A1  | PA0_C  | ADC1_INP0 | 第1路火焰传感器 |
| A2  | PA1_C  | ADC1_INP1 | 第2路火焰传感器 |
| A3  | PA6    | ADC1_INP3 | 第3路火焰传感器 |
| A4  | PC4    | ADC1_INP4 | 第4路火焰传感器 |
| A5  | PB1    | ADC1_INP5 | 第5路火焰传感器 |

### 代码文件
- `applications/Apps/linesensor.c/h`
- `applications/Apps/hal_adc.c/h`

### 调试步骤

1. **确认硬件连接**
   ```
   传感器供电: VCC → 3.3V/5V (根据传感器要求)
   传感器地:   GND → GND
   模拟输出:   A1-A5 → 对应MCU引脚
   ```

2. **查看线程状态**
   ```bash
   msh> list_thread
   # 应该看到 "adc_read" 线程运行
   ```

3. **查看全局变量（需要添加MSH命令）**
   - 全局变量：`g_adc_ch0` ~ `g_adc_ch5`
   - ADC采样周期：500ms

4. **验证方法**
   - 用打火机或蜡烛在传感器前晃动
   - ADC值应该明显变化（0 ~ 4095范围）
   - 无火焰时：ADC值较低（< 500）
   - 有火焰时：ADC值较高（> 2000）

### 预期输出

```
[LineSensor] Five-channel flame sensor ADC read thread started (A1-A5, HAL mode).
```

---

## 2️⃣ 温湿度传感器（SHT30）

### 硬件接口

| 信号 | MCU引脚 | 说明 |
|-----|--------|------|
| SCL | PB6    | I2C1时钟线 |
| SDA | PB7    | I2C1数据线 |
| VCC | 3.3V   | 供电 |
| GND | GND    | 地 |

- **I2C地址：** 0x44
- **总线：** i2c1

### 代码文件
- `applications/Apps/sht30App.c/h`

### 调试步骤

1. **确认I2C总线**
   ```bash
   msh> list_device
   # 应该看到 "i2c1" 设备
   ```

2. **查看初始化日志**
   ```
   [main] I: SHT30 initialized on i2c1
   [SHT30] SHT30 thread started on i2c1
   ```

3. **查看线程状态**
   ```bash
   msh> list_thread
   # 应该看到 "sht30_rd" 线程运行
   ```

4. **验证数据**
   - 全局变量：`g_temperature_c`（温度，℃）
   - 全局变量：`g_humidity_rh`（湿度，%RH）
   - 常温下：温度约20-30℃，湿度约40-60%

5. **CRC校验**
   - SHT30使用CRC8校验确保数据完整性
   - 如果CRC失败，日志会输出错误信息

### 预期输出

```
[main] I: SHT30 initialized on i2c1
[SHT30] SHT30 thread started on i2c1
```

### 故障排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| I2C bus not found | I2C1未初始化 | 检查board.c中的I2C配置 |
| CRC8 fail | 数据传输错误 | 检查I2C接线，降低SCL频率 |
| 读数异常 | 供电不稳定 | 检查3.3V电源，加旁路电容 |

---

## 3️⃣ 甲烷气体传感器

### 硬件接口

| 信号 | MCU引脚 | 说明 |
|-----|--------|------|
| TX  | PD5    | UART2发送 |
| RX  | PD6    | UART2接收 |
| VCC | 3.3V   | 供电 |
| GND | GND    | 地 |

- **波特率：** 19200
- **数据位：** 8
- **停止位：** 1
- **校验位：** None

### 代码文件
- `applications/Apps/MethaneSensorApp.c/h`

### 数据帧格式

```
AC AC 13 AA [data...] [checksum]
│  │  │  │
│  │  │  └─ 帧标记
│  │  └──── 数据长度（19字节）
│  └─────── 帧头
└────────── 帧头

PPM值 = (byte[11] << 8) | byte[10]
LEL%  = byte[12]
Alarm = byte[13]
```

### 调试步骤

1. **查看线程状态**
   ```bash
   msh> list_thread
   # 应该看到 "uart2_rx" 线程运行
   ```

2. **查看全局变量**
   - `g_methane_ppm`：甲烷浓度（ppm）
   - `g_methane_lel`：爆炸下限百分比（%LEL）

3. **启用调试输出**
   - 在`MethaneSensorApp.c`中取消注释`rt_kprintf`语句
   - 可以看到原始十六进制数据和解析结果

4. **验证方法**
   - 正常空气中：PPM值应接近0
   - 用打火机或煤气靠近传感器
   - PPM值应明显上升

### 预期输出

```
gas:XXX.XXX ppm, levle: XX%, alarm:0xXX
```

---

## 4️⃣ 氧气传感器（O2）

### 硬件接口

| 信号 | MCU引脚 | ADC通道 | 说明 |
|-----|--------|---------|-----|
| AOUT | PA4    | ADC1_CH18 | 模拟输出 |
| VCC  | 3.3V/5V | -      | 供电 |
| GND  | GND    | -       | 地 |

### 代码文件
- `applications/Apps/o2SensorApp.c/h`

### 转换公式

```c
mV = raw * 3300 / 4096  // 12位ADC，3.3V参考电压

O2%(×10) = (mV - 400) * 209 / (2000 - 400)

// 空气稳定化：19.5% ~ 22.0% → 强制20.9%
if (O2% >= 19.5 && O2% <= 22.0)
    O2% = 20.9%
```

### 校准参数

```c
#define O2_ZERO_OFFSET_MV      400   // 0% O2对应的mV
#define O2_FULL_SCALE_MV       2000  // 20.9% O2对应的mV
```

### 调试步骤

1. **查看线程状态**
   ```bash
   msh> list_thread
   # 应该看到 "o2_rd" 线程运行
   ```

2. **查看初始化日志**
   ```
   [main] I: O2 sensor initialized on adc1 ch18
   [O2] O2 thread started on adc1 ch18
   ```

3. **验证数据**
   - 全局变量：`g_o2_concentration`（氧气浓度，%）
   - 空气中：应读取约20.9%
   - 密闭容器：氧气浓度下降

4. **校准方法**
   - 在正常空气中读取ADC原始值
   - 如果读数偏离20.9%，调整`O2_FULL_SCALE_MV`

### 预期输出

```
[main] I: O2 sensor initialized on adc1 ch18
[O2] O2 thread started on adc1 ch18
```

---

## 5️⃣ 位移传感器（RS485 Modbus）

### 硬件接口

| 信号 | 接口 | 说明 |
|-----|------|------|
| A   | CN2.1 | 485差分信号+ |
| B   | CN2.2 | 485差分信号- |
| +18V | CN2.5 | 供电正极 |
| GND  | CN2.4 | 供电负极 |

- **协议：** Modbus RTU
- **UART：** UART3 (PB10/PB11)
- **波特率：** 9600
- **从站地址：** 0x01

### 代码文件
- `applications/Apps/freeModbusApp.c`
- `applications/Apps/displacementSensorApp.c/h`

### Modbus参数

```c
#define DISPLACEMENT_SLAVE_ADDR    0x01    // 从站地址
#define DISPLACEMENT_REG_START     0x0001  // 起始寄存器
#define DISPLACEMENT_REG_NUM       2       // 寄存器数量
```

### 转换公式

```c
adjusted_raw = raw_value - 74  // 零点偏移
Displacement(mm) = adjusted_raw * 0.025  // 比例因子
```

### 调试步骤

1. **查看Modbus线程**
   ```bash
   msh> list_thread
   # 应该看到 "md_m_poll" 和 "md_m_send" 线程
   ```

2. **启用调试输出**
   - 在`freeModbusApp.c`中取消注释位移传感器的`rt_kprintf`
   ```c
   rt_kprintf("[Displacement] Value: %d.%03d mm (Raw: %d)\n", ...);
   ```

3. **验证数据**
   - 全局变量：`Displacement`（mm）
   - 节点数据：`node[0].Displacement`（mm）
   - 手动改变位移，读数应相应变化

4. **零点校准**
   - 记录无位移时的原始值
   - 调整`adjusted_raw = raw_value - 零点值`

### 预期输出

```
[Displacement] Value: XXX.XXX mm (Raw: XXXX)
```

### 故障排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| Modbus超时 | 接线错误 | 检查A/B接线，交换试试 |
| 读数为0 | 从站地址错误 | 用Modbus工具扫描地址 |
| 读数跳变 | 总线干扰 | 检查接地，加终端电阻 |

---

## 6️⃣ 应力传感器（RDF-TC25/RDD-DH）

应力传感器已完成调试并集成到 UART3 Modbus 主轮询。生产参数为
`9600-8-N-1`、从站地址 `3`、单位 `N`、两位小数。

### 硬件接口

| 信号 | 接口 | 线色 | 说明 |
|-----|------|------|------|
| +18V | CN2.5 | 红线 | 电源正极 |
| GND  | CN2.4 | 白线 | 电源负极 |
| 485A | CN2.1 | 蓝线 | 差分信号+ |
| 485B | CN2.2 | 绿线 | 差分信号- |

- **协议：** Modbus RTU
- **UART：** UART3 (PB10/PB11) - 共享总线
- **波特率：** 9600
- **从站地址：** 3
- **量程：** 0~400kN
- **精度：** ±4% FS
- **工作电流：** ≤60mA

### 代码文件
- `applications/Apps/stressSensorApp.c/h`

### 运行方式

上电后由 `freeModbusApp.c` 自动轮询。使用通用 `modbus` 命令查看在线状态和当前值；
生产固件不再提供应力传感器专用扫描、改址、改单位或测试线程。

### 全局变量
- `g_stress_value_n`：当前力值（N）
- `g_stress_sensor_online`：传感器在线状态

### 故障排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 连续5次失败 | 通信参数或接线错误 | 核对9600-8-N-1及A/B接线 |
| Modbus超时 | 从站地址错误 | 使用独立Modbus工具确认地址3 |
| 读数异常 | 数据解析错误 | 检查原始寄存器值，调整转换公式 |
| 总线冲突 | 多设备共用485 | 逐个连接测试，确认地址不冲突 |

---

## 7️⃣ 电表（三相电流表）

### 硬件接口

- **协议：** Modbus RTU
- **UART：** UART3 (PB10/PB11) - 共享总线
- **波特率：** 9600
- **从站地址：** 73 (0x49)

### 代码文件
- `applications/Apps/freeModbusApp.c/h`

### Modbus参数

```c
#define AMMETER_SLAVE_ADDR     73      // 从站地址
#define VOLTAGE_REG_START      0x0100  // 电压寄存器起始地址
#define VOLTAGE_REG_NUM        6       // 3相电压，每相2个寄存器
#define CURRENT_REG_START      0x010E  // 电流寄存器起始地址
#define CURRENT_REG_NUM        6       // 3相电流，每相2个寄存器
```

### 数据格式

- 每个浮点数占用2个16位寄存器（大端序）
- 高16位在前，低16位在后
- IEEE 754浮点格式

### 转换公式

```c
uint32_t combined = (reg_high << 16) | reg_low;
memcpy(&float_value, &combined, sizeof(float));
```

### 调试步骤

1. **启用调试输出**
   - 在`freeModbusApp.c`中取消注释电表的`rt_kprintf`

2. **验证数据**
   - 全局变量：`Voltage[3]`（三相电压，V）
   - 全局变量：`Current[3]`（三相电流，A）
   - 节点数据：`node[0].CH1_A[3]`（电流数组）

3. **读取顺序**
   - 先读电压（0x0100开始，6个寄存器）
   - 再读电流（0x010E开始，6个寄存器）

### 预期输出

```
[Ammeter] Voltage[0]: XXX.XX V (Reg[0x100]=0xXXXX, Reg[0x101]=0xXXXX)
[Ammeter] Voltage[1]: XXX.XX V (Reg[0x102]=0xXXXX, Reg[0x103]=0xXXXX)
[Ammeter] Voltage[2]: XXX.XX V (Reg[0x104]=0xXXXX, Reg[0x105]=0xXXXX)
[Ammeter] Current[0]: XX.XX A (Reg[0x10E]=0xXXXX, Reg[0x10F]=0xXXXX)
[Ammeter] Current[1]: XX.XX A (Reg[0x110]=0xXXXX, Reg[0x111]=0xXXXX)
[Ammeter] Current[2]: XX.XX A (Reg[0x112]=0xXXXX, Reg[0x113]=0xXXXX)
```

### 注意事项

⚠️ **从站地址73需要`MB_MASTER_TOTAL_SLAVE_NUM >= 73`**

检查`rtconfig.h`或`freeModbus`配置：
```c
#define MB_MASTER_TOTAL_SLAVE_NUM  128  // 确保≥73
```

---

## 📊 数据汇总与上报

所有传感器数据最终汇总到：

1. **节点数组：** `node[NODENUM]`
2. **全局变量：** 各传感器的`g_xxx`变量
3. **上报通道：**
   - 华为云MQTT（WiFi/4G）
   - LoRa无线传输
   - 以太网TCP/IP
   - Modbus RTU（作为从站）

---

## 🔍 调试工具命令

### MSH Shell命令

```bash
# 查看线程列表
list_thread

# 查看设备列表
list_device

```

---

## 📝 调试记录模板

| 传感器 | 日期 | 状态 | 读数示例 | 问题 | 解决方案 |
|-------|------|------|---------|------|---------|
| 火焰传感器 | | ☐ | ADC: 0~4095 | | |
| SHT30温湿度 | | ☐ | 25.3℃, 55%RH | | |
| 甲烷传感器 | | ☐ | 120 ppm | | |
| 氧气传感器 | | ☐ | 20.9% | | |
| 位移传感器 | | ☐ | 12.5 mm | | |
| 应力传感器 | | ☐ | 85.6 kN | | |
| 电表 | | ☐ | 220V, 5.2A | | |

---

## 🛠️ 常见问题汇总

### Modbus通信失败
- 检查A/B接线（交换试试）
- 检查从站地址
- 检查波特率匹配
- 检查供电电压（18V）
- 添加120Ω终端电阻

### ADC读数异常
- 检查供电电压（3.3V/5V）
- 检查引脚配置
- 检查参考电压
- 添加RC滤波电路

### I2C通信失败
- 检查SCL/SDA接线
- 检查I2C地址（0x44）
- 检查上拉电阻（4.7kΩ）
- 降低SCL频率

### UART接收不到数据
- 检查TX/RX是否交叉连接
- 检查波特率配置
- 检查供电是否稳定
- 用示波器查看波形

---

## 📖 参考文档

- [STM32H743XI数据手册](https://www.st.com/resource/en/datasheet/stm32h743xi.pdf)
- [RT-Thread文档中心](https://www.rt-thread.org/document/site/)
- [FreeModbus协议栈](https://github.com/armink/FreeModbus_Slave-Master-RTT-STM32)
- [SHT30传感器手册](https://www.sensirion.com/en/environmental-sensors/humidity-sensors/digital-humidity-sensors-for-various-applications/)

---

## 📅 更新记录

| 日期 | 版本 | 修改内容 | 修改人 |
|-----|------|---------|--------|
| 2026-09-18 | v1.0 | 初始版本，完整调试指南 | ideapad15s |
