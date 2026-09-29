# 截图指引

## 截图内容建议

根据操作手册的章节，建议截取以下内容：

1. **系统启动与初始化**
   - 串口终端显示 RT-Thread 启动日志
   - 显示内核版本号和线程创建成功的提示

2. **甲烷浓度监测**
   - 串口输出甲烷浓度 PPM 值和 LEL 百分比
   - 例如：`gas:XXX.X ppm, level: XX%, alarm:0xXX`

3. **火焰传感器监测**
   - 串口输出五路 ADC 通道数据
   - 例如：`ADC CH0: XXXXX CH1: XXX CH3: XXX CH4: XXX CH5: XXX`

4. **氧气浓度监测**
   - 串口输出氧气浓度百分比
   - 例如：`O2: 20.9%`

5. **温湿度监测**
   - 串口输出温湿度数据
   - 例如：`Temperature: XX.X C, Humidity: XX.X %RH`

6. **电表数据采集**
   - 串口输出三相电压和电流
   - 例如：`[Ammeter] Voltage[0]: XXX.XX V, Current[0]: X.XXX A`

7. **水表流量采集**
   - 串口输出水流量值
   - 例如：`[Water] Flow: X.XXX`

8. **应力传感器监测**
   - 串口输出应力值
   - 例如：`Stress: X.XX N`

9. **以太网数据上报**
   - 串口显示网络连接状态和 TCP 连接日志
   - 例如：`connected to 192.168.1.100:8080` 和 `Sent N bytes via ETH`
   - （可选）监控中心服务器接收数据的截图

10. **串口调试与状态查看**
    - MSH 命令行界面
    - 例如：`msh >` 提示符以及 `list_thread`、`list_device`、`sensors` 等命令的输出

## 文件命名建议

请按顺序给截图文件添加数字前缀，例如：

```
1-系统启动.png
2-甲烷浓度.png
3-火焰传感器.png
4-氧气浓度.png
5-温湿度.png
6-电表数据.png
7-水表流量.png
8-应力传感器.png
9-以太网上报.png
10-串口调试.png
```

支持的图片格式：PNG、JPG、JPEG、WebP

## 截图工具

- Windows：使用 Windows 截图工具（Win + Shift + S）或 Snipping Tool
- 串口终端软件：PuTTY、SecureCRT 等都支持直接截图

## 完成后

将截图文件放入本目录（`软件著作权申请资料/用户截图/`）后，告诉我"截图已准备好"，我会整理截图并继续生成正式资料。

如果暂时无法截图，也可以选择"先跳过截图"，操作手册会保留截图预留位置，后续可以补充。
