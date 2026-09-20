# 传感器调试项目 - README

## 📚 文档索引

本次为传感器逐个调试创建了完整的文档和工具集：

### 📖 主要文档

1. **[SENSOR_DEBUG_GUIDE.md](SENSOR_DEBUG_GUIDE.md)** - 详细调试指南
   - 完整的7个传感器调试步骤
   - 硬件接口定义
   - 代码文件说明
   - 调试方法和验证步骤
   - 故障排查方案

2. **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - 快速参考卡
   - MSH命令速查表
   - 逐个传感器调试流程
   - 常见问题快速修复
   - 调试检查清单

3. **[SUMMARY.md](SUMMARY.md)** - 项目总结
   - 传感器实现状态汇总
   - 关键问题与解决方案
   - 全局变量汇总
   - 新增文件列表

---

## 🎯 快速开始

### 1. 查看传感器状态
所有传感器已实现，按以下顺序调试：

```
1. 火焰传感器（5通道ADC）       ✅ 已完成
2. 温湿度传感器（SHT30）        ✅ 已完成
3. 甲烷气体传感器              ✅ 已完成
4. 氧气传感器（O2）            ✅ 已完成
5. 位移传感器（Modbus）         ✅ 已完成
6. 应力传感器（GMY400）         ⚠️ 波特率问题
7. 电表（三相，Modbus）         ✅ 已完成
```

### 2. 编译项目
```bash
cd /c/Users/ideapad15s/Desktop/Pipe_gallery_node
scons
```

### 3. 烧录并连接串口
- 波特率：115200
- 使用 PuTTY/SecureCRT 等工具

### 4. 使用调试命令
```bash
msh> sensors          # 查看所有传感器
msh> flame            # 查看火焰传感器
msh> env              # 查看环境传感器
msh> modbus           # 查看Modbus设备
msh> monitor 1        # 实时监控（每秒刷新）
```

---

## 🔧 新增工具

### MSH调试命令（debugCommands.c）
- `sensors` - 显示所有传感器数据汇总
- `flame` - 显示火焰传感器详细状态
- `env` - 显示环境传感器
- `modbus` - 显示Modbus设备状态
- `monitor [sec]` - 实时监控
- `test <sensor>` - 测试单个传感器

### 调试配置（sensor_debug_config.h）
- 分阶段调试开关
- 调试输出控制
- 采样周期配置

---

## ⚠️ 关键问题

### 应力传感器 RDF-TC25/RDD-DH

设备已配置为 `9600-8-N-1`、从站地址 `3`、单位 `N`。生产固件不再提供应力传感器
专用配置或扫描命令，启动后由 UART3 Modbus 线程自动轮询。

---

## 📁 文件结构

```
Pipe_gallery_node/
├── applications/
│   ├── Apps/
│   │   ├── debugCommands.c/h          # 新增：MSH调试命令
│   │   ├── sensor_debug_config.h      # 新增：调试配置
│   │   ├── stressSensorApp.c/h        # 应力传感器（已有）
│   │   ├── linesensor.c/h             # 火焰传感器
│   │   ├── sht30App.c/h               # 温湿度传感器
│   │   ├── MethaneSensorApp.c/h       # 甲烷传感器
│   │   ├── o2SensorApp.c/h            # 氧气传感器
│   │   ├── displacementSensorApp.c/h  # 位移传感器
│   │   └── freeModbusApp.c/h          # Modbus主站
│   └── main.c                          # 已修改：添加头文件
├── docs/
│   ├── SENSOR_DEBUG_GUIDE.md          # 新增：详细调试指南
│   ├── QUICK_REFERENCE.md             # 新增：快速参考
│   ├── SUMMARY.md                     # 新增：项目总结
│   └── README.md                      # 本文件
└── Netlist_Schematic1_2026-09-18.tel  # 原理图网表
```

---

## 📊 传感器接口汇总

| 传感器 | 接口类型 | 引脚/地址 | 波特率 |
|--------|---------|----------|--------|
| 火焰传感器×5 | ADC | PA0_C, PA1_C, PA6, PC4, PB1 | - |
| SHT30 温湿度 | I2C1 | PB6(SCL), PB7(SDA) | - |
| 甲烷传感器 | UART2 | PD5(TX), PD6(RX) | 19200 |
| 氧气传感器 | ADC1 CH18 | PA4 | - |
| 位移传感器 | RS485 Modbus | CN2 (UART3) | 9600 |
| 应力传感器 | RS485 Modbus | CN2 (UART3) | 2400/4800⚠️ |
| 电表 | RS485 Modbus | CN2 (UART3) | 9600 |

---

## 🚀 开发建议

### 调试顺序
1. 先调试独立传感器（火焰、温湿度、甲烷、氧气）
2. 再调试Modbus设备（位移、应力、电表）
3. 单独测试每个Modbus设备（避免地址冲突）
4. 最后整体联调

### 使用调试阶段宏
```c
// 在 sensor_debug_config.h 中选择调试阶段
#define DEBUG_STAGE_1_FLAME        // 仅启用火焰传感器
// #define DEBUG_STAGE_2_SHT30     // 启用火焰+温湿度
// #define DEBUG_STAGE_ALL_SENSORS // 全功能模式
```

### 启用调试输出
```c
// 在对应传感器代码中设置
#define DBG_LVL DBG_LOG  // 详细日志
```

---

## 📞 获取帮助

1. 查看详细文档：`docs/SENSOR_DEBUG_GUIDE.md`
2. 查看快速参考：`docs/QUICK_REFERENCE.md`
3. 查看代码注释
4. 使用MSH命令诊断

---

## ✅ 下一步

- [ ] 编译并烧录项目
- [ ] 按顺序调试7个传感器
- [ ] 解决应力传感器波特率问题
- [ ] 整体联调测试
- [ ] 长时间稳定性测试

---

**创建日期：** 2026-09-18  
**版本：** v1.0  
**作者：** ideapad15s
