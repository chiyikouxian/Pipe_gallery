# 传感器调试快速参考卡

## 🚀 快速开始

### 1. 编译项目
```bash
cd /c/Users/ideapad15s/Desktop/Pipe_gallery_node
scons
```

### 2. 烧录到STM32
使用ST-Link或J-Link烧录 `rt-thread.elf`

### 3. 连接串口终端
- 波特率：115200
- 数据位：8
- 停止位：1
- 校验位：None
- 终端软件：PuTTY/SecureCRT/XShell

### 4. 查看传感器数据
```bash
msh> sensors          # 显示所有传感器数据
msh> flame            # 显示火焰传感器
msh> env              # 显示环境传感器
msh> modbus           # 显示Modbus设备
msh> monitor 2        # 每2秒刷新监控（按Ctrl+C退出）
```

---

## 📊 MSH命令速查表

| 命令 | 功能 | 示例 |
|-----|------|------|
| `sensors` | 显示所有传感器数据汇总 | `msh> sensors` |
| `flame` | 显示五通道火焰传感器状态 | `msh> flame` |
| `env` | 显示环境传感器（温湿度+O2+甲烷） | `msh> env` |
| `modbus` | 显示Modbus设备状态 | `msh> modbus` |
| `monitor [sec]` | 实时监控（可选刷新间隔秒数） | `msh> monitor 2` |
| `test <sensor>` | 测试单个传感器 | `msh> test flame` |
| `stress_scan_bus` | 扫描Modbus总线（地址1-10） | `msh> stress_scan_bus` |
| `stress_test_addr <addr>` | 测试应力传感器指定地址 | `msh> stress_test_addr 3` |
| `stress_set_addr <old> <new>` | 修改应力传感器从站地址 | `msh> stress_set_addr 1 3` |
| `list_thread` | 显示所有线程 | `msh> list_thread` |
| `list_device` | 显示所有设备 | `msh> list_device` |
| `free` | 显示内存使用情况 | `msh> free` |

---

## 🔍 逐个传感器调试流程

### 第1步：火焰传感器
```bash
# 1. 查看线程
msh> list_thread
# 确认 "adc_read" 线程运行

# 2. 查看数据
msh> flame
# 应该看到5个通道的ADC值

# 3. 测试
用打火机或蜡烛在传感器前晃动，ADC值应该>2000

# 4. 实时监控
msh> monitor 1
```

---

### 第2步：温湿度传感器（SHT30）
```bash
# 1. 查看设备
msh> list_device
# 确认 "i2c1" 存在

# 2. 查看线程
msh> list_thread
# 确认 "sht30_rd" 线程运行

# 3. 查看数据
msh> env
# 应该看到温度和湿度数据

# 4. 验证
常温下：温度约20-30℃，湿度约40-60%RH
```

---

### 第3步：甲烷气体传感器
```bash
# 1. 查看线程
msh> list_thread
# 确认 "uart2_rx" 线程运行

# 2. 查看数据
msh> env
# 应该看到甲烷浓度（PPM）和LEL%

# 3. 测试
正常空气中：PPM接近0
用打火机或煤气靠近：PPM明显上升
```

---

### 第4步：氧气传感器（O2）
```bash
# 1. 查看线程
msh> list_thread
# 确认 "o2_rd" 线程运行

# 2. 查看数据
msh> env
# 应该看到O2浓度约20.9%

# 3. 测试
正常空气：20.9%
密闭容器/呼出气体：降低至18-19%
```

---

### 第5步：位移传感器
```bash
# 1. 查看Modbus线程
msh> list_thread
# 确认 "md_m_poll" 和 "md_m_send" 线程运行

# 2. 查看数据
msh> modbus
# 应该看到位移值（mm）

# 3. 测试
手动改变位移，读数应相应变化

# 4. 故障排查
如果读取失败，检查：
- 485 A/B接线（尝试交换）
- 供电18V是否正常
- 从站地址是否为1
```

---

### 第6步：应力传感器（GMY400）⚠️ 波特率问题

**⚠️ 重要：传感器波特率为2400/4800，系统当前为9600！**

#### 选项A：修改系统波特率为2400（测试用）

1. 修改 `freeModbusApp.h`:
```c
#define MB_MASTER_BAUDRATE  2400  // 改为2400
```

2. 重新编译烧录
```bash
scons -c && scons
```

3. 测试通信
```bash
msh> stress_scan_bus
# 扫描地址1-10，查找响应的地址
```

#### 选项B：扫描总线查找设备
```bash
# 使用当前9600波特率先尝试
msh> stress_scan_bus

# 如果找不到，必须修改为2400/4800再试
```

#### 选项C：修改传感器波特率为9600（生产用）

需要使用手持编程器或专用Modbus工具。

---

### 第7步：电表（三相电流表）
```bash
# 1. 查看数据
msh> modbus
# 应该看到三相电压和电流

# 2. 验证
确认从站地址73的设备正常响应
电压应在220V左右
电流根据实际负载而定

# 3. 注意事项
确保 MB_MASTER_TOTAL_SLAVE_NUM >= 73
```

---

## 🛠️ 常见问题快速修复

### Q1: 编译失败
```bash
# 清理后重新编译
scons -c
scons
```

### Q2: 线程未运行
```bash
# 检查main.c中是否启用了该传感器的初始化代码
# 查看启动日志中的错误信息
```

### Q3: Modbus超时
```bash
# 1. 检查485接线（A/B可能接反）
# 2. 检查从站地址
# 3. 检查波特率
# 4. 检查供电18V
```

### Q4: I2C通信失败
```bash
# 1. 检查SCL/SDA接线
# 2. 检查上拉电阻（4.7kΩ）
# 3. 检查I2C地址（SHT30为0x44）
# 4. 检查供电3.3V
```

### Q5: UART无数据
```bash
# 1. 检查TX/RX是否交叉连接
# 2. 检查波特率配置
# 3. 用示波器查看波形
# 4. 检查供电是否稳定
```

---

## 📁 重要文件位置

### 传感器应用代码
```
applications/Apps/
├── linesensor.c/h          # 火焰传感器
├── sht30App.c/h            # 温湿度传感器
├── MethaneSensorApp.c/h    # 甲烷传感器
├── o2SensorApp.c/h         # 氧气传感器
├── displacementSensorApp.c/h  # 位移传感器
├── stressSensorApp.c/h     # 应力传感器
├── freeModbusApp.c/h       # Modbus主站（电表、位移、应力）
└── debugCommands.c/h       # 调试MSH命令
```

### 配置文件
```
rtconfig.h                  # RT-Thread配置
board.h                     # 板级配置
```

### 调试文档
```
docs/SENSOR_DEBUG_GUIDE.md  # 详细调试指南
docs/QUICK_REFERENCE.md     # 本快速参考（本文件）
```

---

## 🎯 调试检查清单

复制以下清单，每完成一项就打勾：

```
传感器调试进度：

☐ 1. 火焰传感器
    ☐ 硬件接线检查
    ☐ 线程运行确认
    ☐ 数据读取正常
    ☐ 火焰检测测试
    ☐ 长时间稳定性测试

☐ 2. 温湿度传感器（SHT30）
    ☐ I2C总线检查
    ☐ 线程运行确认
    ☐ 温度读数正常
    ☐ 湿度读数正常
    ☐ CRC校验通过

☐ 3. 甲烷气体传感器
    ☐ UART2配置检查
    ☐ 线程运行确认
    ☐ 数据帧解析正常
    ☐ 气体响应测试
    ☐ 报警功能验证

☐ 4. 氧气传感器（O2）
    ☐ ADC通道配置
    ☐ 线程运行确认
    ☐ 空气中校准（20.9%）
    ☐ 低氧响应测试
    ☐ 数据稳定性验证

☐ 5. 位移传感器
    ☐ RS485接线检查
    ☐ Modbus通信正常
    ☐ 零点校准完成
    ☐ 位移响应测试
    ☐ 数据精度验证

☐ 6. 应力传感器（GMY400）
    ☐ 波特率匹配解决
    ☐ 从站地址确认
    ☐ Modbus通信正常
    ☐ 应力读数正常
    ☐ 量程测试（0-400kN）

☐ 7. 电表（三相）
    ☐ Modbus通信正常
    ☐ 三相电压读取
    ☐ 三相电流读取
    ☐ 数据准确性验证
    ☐ 长时间稳定性测试
```

---

## 📞 技术支持

如果遇到问题：

1. 查看 `docs/SENSOR_DEBUG_GUIDE.md` 详细文档
2. 查看代码中的注释说明
3. 使用MSH调试命令排查
4. 检查硬件接线和供电
5. 查看RT-Thread官方文档

---

## 📝 调试日志模板

在调试过程中记录：

```
日期：2026-09-18
传感器：火焰传感器
状态：✅ 通过
读数示例：A1=120, A2=135, A3=98, A4=110, A5=105
问题：无
备注：5个通道均工作正常

---

日期：2026-09-18
传感器：应力传感器GMY400
状态：⚠️ 待解决
读数示例：Offline
问题：波特率不匹配（系统9600 vs 传感器2400/4800）
解决方案：计划使用方案A - 修改系统波特率为2400测试

---
```

---

**祝调试顺利！🎉**
