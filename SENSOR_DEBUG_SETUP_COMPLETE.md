# 📊 传感器调试完整方案 - 项目总结

## 🎯 任务概述

为管廊监测节点项目创建了完整的传感器逐个调试方案，包括详细文档、MSH调试命令和配置工具。

---

## ✅ 已完成工作

### 📖 1. 文档系统（docs/）

创建了完整的调试文档体系：

| 文档 | 用途 | 字数 |
|-----|------|------|
| **SENSOR_DEBUG_GUIDE.md** | 详细调试指南，包含7个传感器的完整调试步骤 | ~8000字 |
| **QUICK_REFERENCE.md** | 快速参考卡，MSH命令速查和常见问题 | ~3000字 |
| **SUMMARY.md** | 项目总结，传感器状态汇总和问题分析 | ~4000字 |
| **README.md** | 文档索引和快速入门 | ~1500字 |

### 🔧 2. 调试工具代码

#### debugCommands.c/h（新增）
提供了丰富的MSH调试命令：

```bash
sensors          # 显示所有传感器数据汇总
flame            # 显示五通道火焰传感器详细状态
env              # 显示环境传感器（温湿度+O2+甲烷）
modbus           # 显示Modbus设备状态
monitor [sec]    # 实时监控传感器数据
test <sensor>    # 测试单个传感器
```

**功能亮点：**
- 格式化数据显示
- 实时监控模式
- 异常数据告警
- 单传感器测试

#### sensor_debug_config.h（新增）
调试配置系统：

```c
// 7个调试阶段开关
DEBUG_STAGE_1_FLAME
DEBUG_STAGE_2_SHT30
DEBUG_STAGE_3_METHANE
DEBUG_STAGE_4_O2
DEBUG_STAGE_5_DISPLACEMENT
DEBUG_STAGE_6_STRESS
DEBUG_STAGE_7_AMMETER
DEBUG_STAGE_ALL_SENSORS  // 生产模式
```

**功能亮点：**
- 分阶段调试控制
- 调试输出开关
- 采样周期配置
- Modbus参数配置

### 🛠️ 3. 辅助脚本

#### check_before_compile.sh（新增）
编译前检查脚本：

- 检查项目结构
- 检查传感器文件
- 检查工具链
- 生成检查报告

### 📝 4. 代码修改

#### applications/main.c
- 添加 `#include "stressSensorApp.h"`
- 为应力传感器集成做准备

---

## 📊 传感器实现状态

| # | 传感器 | 状态 | 接口 | 备注 |
|---|--------|------|------|------|
| 1 | 火焰传感器（5通道） | ✅ 完成 | ADC1×5 | HAL库直接读取 |
| 2 | 温湿度传感器 SHT30 | ✅ 完成 | I2C1 | CRC8校验 |
| 3 | 甲烷气体传感器 | ✅ 完成 | UART2 | 19字节固定帧 |
| 4 | 氧气传感器 O2 | ✅ 完成 | ADC1 CH18 | 线性转换 |
| 5 | 位移传感器 | ✅ 完成 | Modbus RTU | 已集成主轮询 |
| 6 | 应力传感器 GMY400 | ⚠️ 未集成 | Modbus RTU | **波特率不匹配** |
| 7 | 电表（三相） | ✅ 完成 | Modbus RTU | 从站地址73 |

---

## ⚠️ 关键问题识别

### 应力传感器GMY400波特率问题

**问题描述：**
- 传感器标称波特率：2400 或 4800
- 系统当前波特率：9600
- **无法通信，必须解决！**

**已提供的解决方案：**

1. **方案A：修改系统波特率**（测试用）
   ```c
   // 在 freeModbusApp.h 中
   #define MB_MASTER_BAUDRATE  2400
   ```

2. **方案B：使用MSH命令扫描**
   ```bash
   msh> stress_scan_bus        # 扫描地址1-10
   msh> stress_test_addr 3     # 测试指定地址
   ```

3. **方案C：修改传感器波特率**（生产用）
   ```bash
   msh> stress_set_addr 1 3    # 修改从站地址
   ```

**文档位置：**
- `docs/SENSOR_DEBUG_GUIDE.md` 第6节
- `docs/SUMMARY.md` 问题1节

---

## 🎓 调试顺序建议

按以下顺序逐个调试（已在文档中详细说明）：

```
第1步：火焰传感器（5通道ADC）
  └─ 硬件简单，独立工作，适合第一个测试

第2步：温湿度传感器（SHT30）
  └─ I2C通信，有CRC校验，验证I2C总线

第3步：甲烷气体传感器
  └─ UART通信，固定帧格式，验证UART2

第4步：氧气传感器
  └─ ADC采集，线性转换，验证ADC1另一通道

第5步：位移传感器
  └─ Modbus通信，先测试简单设备

第6步：应力传感器⚠️
  └─ Modbus通信，解决波特率问题

第7步：电表（三相）
  └─ Modbus通信，验证完整系统
```

---

## 📁 文件清单

### 新增文件（7个）

```
docs/
├── SENSOR_DEBUG_GUIDE.md      # 详细调试指南
├── QUICK_REFERENCE.md         # 快速参考卡
├── SUMMARY.md                 # 项目总结
└── README.md                  # 文档索引

applications/Apps/
├── debugCommands.c            # MSH调试命令实现
├── debugCommands.h            # MSH调试命令头文件
└── sensor_debug_config.h      # 传感器调试配置

scripts/
└── check_before_compile.sh    # 编译前检查脚本
```

### 修改文件（1个）

```
applications/main.c            # 添加 stressSensorApp.h 头文件
```

---

## 🚀 使用指南

### 1. 查看文档
```bash
# 详细调试步骤
cat docs/SENSOR_DEBUG_GUIDE.md

# 快速命令参考
cat docs/QUICK_REFERENCE.md

# 项目总结
cat docs/SUMMARY.md
```

### 2. 编译项目
```bash
# 运行检查脚本（可选）
bash scripts/check_before_compile.sh

# 编译
scons
```

### 3. 烧录调试
```
1. 烧录 rt-thread.elf 到 STM32H743XI
2. 连接串口（115200波特率）
3. 复位MCU，查看启动日志
4. 使用MSH命令调试
```

### 4. 使用调试命令
```bash
msh> sensors          # 查看所有传感器
msh> flame            # 查看火焰传感器
msh> env              # 查看环境传感器
msh> modbus           # 查看Modbus设备
msh> monitor 2        # 每2秒刷新监控
msh> test flame       # 测试火焰传感器
```

---

## 📊 全局变量速查

所有传感器数据都存储在全局变量中，方便调试：

```c
// 火焰传感器
extern rt_uint32_t g_adc_ch0, g_adc_ch1, g_adc_ch3, g_adc_ch4, g_adc_ch5;
extern BOOL Flame;

// 温湿度
extern float g_temperature_c;      // °C
extern float g_humidity_rh;        // %RH

// 甲烷
extern rt_uint16_t g_methane_ppm;  // ppm
extern rt_uint8_t g_methane_lel;   // %LEL

// 氧气
extern float g_o2_concentration;   // %

// 位移
extern float Displacement;         // mm

// 应力
extern float g_stress_value_kn;           // kN
extern rt_bool_t g_stress_sensor_online; // 在线状态

// 电表
extern float Voltage[3];  // V
extern float Current[3];  // A

// 节点数据
extern Node node[NODENUM];
```

---

## ✅ 验收标准

### 必须完成
- [x] 创建完整调试文档体系
- [x] 实现MSH调试命令
- [x] 识别并记录关键问题
- [x] 提供解决方案
- [x] 代码可编译（待验证）

### 待用户完成
- [ ] 编译并烧录项目
- [ ] 逐个调试7个传感器
- [ ] 解决应力传感器波特率问题
- [ ] 整体联调测试
- [ ] 长期稳定性测试

---

## 🎯 下一步行动

### 立即行动
1. **编译验证**
   ```bash
   scons
   ```

2. **烧录测试**
   - 烧录 rt-thread.elf
   - 连接串口终端

3. **测试MSH命令**
   ```bash
   msh> sensors
   ```

### 逐个调试
按文档顺序调试7个传感器，重点解决应力传感器波特率问题。

### 集成测试
所有传感器调试完成后，进行：
- 多传感器同时运行测试
- 数据上报测试（WiFi/LoRa/Ethernet）
- 长时间稳定性测试

---

## 📞 参考资源

### 项目内文档
- `docs/SENSOR_DEBUG_GUIDE.md` - 最详细的调试指南
- `docs/QUICK_REFERENCE.md` - 最常用的命令参考
- `docs/SUMMARY.md` - 项目状态总结
- `README.md` - 原项目说明（已保留）

### 代码注释
所有传感器应用代码中都有详细的中文注释。

### 外部资源
- RT-Thread官方文档
- STM32H743参考手册
- 各传感器数据手册

---

## 📅 版本信息

- **创建日期：** 2026-09-18
- **版本：** v1.0
- **作者：** ideapad15s（AI辅助）
- **项目：** Pipe_gallery_node（管廊监测节点）

---

## 🎉 总结

本次任务为传感器逐个调试创建了**完整的工具链和文档体系**：

✅ **4份详细文档**（共16000+字）  
✅ **丰富的MSH调试命令**  
✅ **灵活的调试配置系统**  
✅ **识别并提供解决方案**（应力传感器波特率问题）  
✅ **清晰的调试顺序和验收标准**

现在你可以：
1. 查看文档了解每个传感器的调试方法
2. 使用MSH命令实时查看所有传感器数据
3. 按照顺序逐个调试，遇到问题参考文档
4. 使用配置系统控制调试阶段

**祝调试顺利！** 🚀
