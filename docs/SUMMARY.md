# 传感器调试总结文档

## 📋 项目状态总览

### 系统信息
- **MCU:** STM32H743XI (ARM Cortex-M7)
- **RTOS:** RT-Thread v4.0.3
- **构建系统:** SCons
- **调试接口:** UART1 (115200 baud)

### 原理图版本
- **最新版本:** Netlist_Schematic1_2026-09-18.tel
- **关键接口:**
  - CN2: 5针2.5mm接口 (RS485 + 18V供电)
  - U7: 2针2.5mm接口 (18V电源输入)
  - 多个ADC通道、I2C、UART接口

---

## ✅ 传感器实现状态

| # | 传感器 | 状态 | 接口 | 代码文件 | 备注 |
|---|--------|------|------|----------|------|
| 1 | 火焰传感器（5通道） | ✅ 完成 | ADC1 (5通道) | linesensor.c/h | HAL库直接读取 |
| 2 | 温湿度传感器 SHT30 | ✅ 完成 | I2C1 (PB6/PB7) | sht30App.c/h | CRC8校验 |
| 3 | 甲烷气体传感器 | ✅ 完成 | UART2 (19200) | MethaneSensorApp.c/h | 19字节固定帧 |
| 4 | 氧气传感器 O2 | ✅ 完成 | ADC1 CH18 (PA4) | o2SensorApp.c/h | 线性转换+空气稳定化 |
| 5 | 位移传感器 | ✅ 完成 | RS485 Modbus | displacementSensorApp.c/h | 已集成到主轮询 |
| 6 | 应力传感器 GMY400 | ⚠️ 未集成 | RS485 Modbus | stressSensorApp.c/h | 波特率不匹配问题 |
| 7 | 电表（三相） | ✅ 完成 | RS485 Modbus | freeModbusApp.c/h | 从站地址73 |

---

## 🔧 已添加的调试工具

### 1. MSH调试命令 (debugCommands.c)
```bash
sensors          # 显示所有传感器数据汇总
flame            # 显示火焰传感器详细状态
env              # 显示环境传感器（温湿度、O2、甲烷）
modbus           # 显示Modbus设备状态
monitor [sec]    # 实时监控传感器数据
test <sensor>    # 测试单个传感器
```

### 2. 调试配置文件 (sensor_debug_config.h)
- 分阶段调试开关
- 调试输出控制
- 采样周期配置
- Modbus参数配置

---

## ⚠️ 关键问题与解决方案

### 应力传感器状态

RDF-TC25/RDD-DH 已集成到 UART3 主轮询，使用 `9600-8-N-1`、地址 `3` 和单位 `N`。
生产固件不再包含专用测试线程和配置命令。

### 问题3: 多个Modbus设备共享总线

**设备列表:**
- 水表: 地址1（未调试，仅作参考）
- 位移传感器: 地址1
- 应力传感器GMY400: 建议地址3
- 电表: 地址73

**注意事项:**
- 避免地址冲突
- 按优先级轮询
- 合理设置超时时间
- 单独测试时断开其他设备

---

## 📊 全局变量汇总

### 火焰传感器
```c
extern rt_uint32_t g_adc_ch0;  // A1通道ADC值
extern rt_uint32_t g_adc_ch1;  // A2通道ADC值
extern rt_uint32_t g_adc_ch3;  // A3通道ADC值
extern rt_uint32_t g_adc_ch4;  // A4通道ADC值
extern rt_uint32_t g_adc_ch5;  // A5通道ADC值
extern BOOL Flame;             // 火焰检测标志
```

### 温湿度传感器 (SHT30)
```c
extern float g_temperature_c;  // 温度 (°C)
extern float g_humidity_rh;    // 湿度 (%RH)
```

### 甲烷传感器
```c
extern rt_uint16_t g_methane_ppm;  // 甲烷浓度 (ppm)
extern rt_uint8_t g_methane_lel;   // 爆炸下限百分比 (%LEL)
```

### 氧气传感器
```c
extern float g_o2_concentration;   // 氧气浓度 (%)
```

### 位移传感器
```c
extern float Displacement;         // 位移值 (mm)
```

### 应力传感器 (RDF-TC25/RDD-DH)
```c
extern float g_stress_value_n;            // 力值 (N)
extern rt_bool_t g_stress_sensor_online;  // 在线状态
```

### 电表（三相）
```c
extern float Voltage[3];  // 三相电压 (V)
extern float Current[3];  // 三相电流 (A)
```

### 节点数据结构
```c
typedef struct Node {
    float CH1_A[3];      // 三相电流
    float Flow;          // 流量
    char Flame;          // 火焰传感器
    float Methane;       // 甲烷浓度
    float Displacement;  // 位移值
} Node;

extern Node node[NODENUM];  // NODENUM = 2
```

---

## 📁 新增文件列表

### 文档
```
docs/
├── SENSOR_DEBUG_GUIDE.md      # 详细调试指南（完整版）
├── QUICK_REFERENCE.md         # 快速参考卡
└── SUMMARY.md                 # 本总结文档
```

### 代码
```
applications/Apps/
├── debugCommands.c            # MSH调试命令实现
├── debugCommands.h            # MSH调试命令头文件
└── sensor_debug_config.h      # 传感器调试配置
```

### 已修改的文件
```
applications/main.c            # 添加了stressSensorApp.h头文件
```

---

## 🚀 下一步行动计划

### 立即行动
1. ✅ 编译项目验证所有代码
2. ✅ 烧录到STM32
3. ✅ 连接串口终端测试MSH命令

### 按顺序调试传感器
1. **火焰传感器** - 用打火机测试
2. **温湿度传感器** - 验证I2C通信
3. **甲烷传感器** - 验证UART2通信
4. **氧气传感器** - 空气中校准
5. **位移传感器** - 验证Modbus通信
6. **应力传感器** - 解决波特率问题
7. **电表** - 验证三相数据读取

### 集成测试
1. 所有传感器同时运行稳定性测试
2. 数据上报测试（WiFi/LoRa/Ethernet）
3. 长时间运行测试（24小时以上）
4. 极端环境测试

---

## 📝 调试技巧

### 技巧1: 分阶段调试
使用 `sensor_debug_config.h` 中的调试阶段宏:
```c
// 只调试火焰传感器
#define DEBUG_STAGE_1_FLAME

// 逐步添加传感器
#define DEBUG_STAGE_2_SHT30
...
```

### 技巧2: 启用调试输出
```c
// 在对应的传感器应用代码中
#define DBG_LVL DBG_LOG  // 改为 DBG_LOG 启用详细输出
```

### 技巧3: 使用实时监控
```bash
msh> monitor 1    # 每秒刷新一次，直观看到所有传感器数据
```

### 技巧4: 单独测试Modbus设备
- 每次只连接一个Modbus设备
- 确认通信正常后再连接下一个
- 避免地址冲突和总线干扰

### 技巧5: 波形分析
- 使用示波器查看UART/I2C/RS485波形
- 确认电压幅度、波特率、时序正确
- 检查信号完整性和干扰

---

## 🔍 故障排查流程图

```
传感器无数据
    │
    ├─ 检查硬件接线
    │   ├─ 供电是否正常？
    │   ├─ 信号线是否接对？
    │   └─ 接地是否良好？
    │
    ├─ 检查软件配置
    │   ├─ 线程是否运行？ (list_thread)
    │   ├─ 设备是否初始化？ (list_device)
    │   └─ 波特率/地址是否正确？
    │
    ├─ 检查通信协议
    │   ├─ UART: TX/RX是否交叉？
    │   ├─ I2C: SCL/SDA是否有上拉？
    │   └─ RS485: A/B是否接反？
    │
    └─ 使用调试工具
        ├─ 示波器查看波形
        ├─ 逻辑分析仪抓取数据
        └─ MSH命令查看实时数据
```

---

## 📞 支持资源

### 官方文档
- [RT-Thread文档中心](https://www.rt-thread.org/document/site/)
- [STM32H743参考手册](https://www.st.com/resource/en/reference_manual/rm0433-stm32h742-stm32h743753-and-stm32h750-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [FreeModbus项目](https://github.com/armink/FreeModbus_Slave-Master-RTT-STM32)

### 传感器手册
- SHT30温湿度传感器数据手册
- GMY400应力传感器说明书
- 位移传感器Modbus协议
- 电表通信协议文档

---

## 📅 版本记录

| 版本 | 日期 | 修改内容 | 修改人 |
|-----|------|---------|--------|
| v1.0 | 2026-09-18 | 创建传感器调试完整方案 | ideapad15s |
| | | - 添加MSH调试命令 | |
| | | - 添加调试配置文件 | |
| | | - 添加详细调试文档 | |
| | | - 识别并记录应力传感器波特率问题 | |

---

## ✅ 完成检查清单

项目准备:
- ✅ 阅读原理图网表 (Netlist_Schematic1_2026-09-18.tel)
- ✅ 分析现有代码结构
- ✅ 识别所有传感器实现状态
- ✅ 发现关键问题（应力传感器波特率）

文档创建:
- ✅ SENSOR_DEBUG_GUIDE.md (详细调试指南)
- ✅ QUICK_REFERENCE.md (快速参考卡)
- ✅ SUMMARY.md (本总结文档)

代码添加:
- ✅ debugCommands.c/h (MSH调试命令)
- ✅ sensor_debug_config.h (调试配置)
- ✅ 修改main.c (添加stressSensorApp.h头文件)

待完成:
- ☐ 编译验证
- ☐ 烧录测试
- ☐ 逐个传感器调试
- ☐ 解决应力传感器波特率问题
- ☐ 集成测试
- ☐ 记录调试结果

---

**祝调试顺利！如有问题，请参考详细文档或查看代码注释。** 🎉
