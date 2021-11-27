# ZMOD4410

## 简介

zmod4410 软件包使用了空气质量传感器 `zmod4410` 的基本功能。

传感器 `zmod4410` 的输入电压为 `1.7V ~ 3.6V` 范围内，详细功能如下表所示：

| 功能 | 量程 | 精度 |
| ---- | ---- | ---- |
| EtOH |  |  |
| TVOC |  |  |
| eCO2 |  |  |
| IAQ  |  |  |

## 支持情况

| 包含设备 | EtOH | TVOC | eCO2 | IAQ |
| ------- | ---- | ---- |---- | ---- |
| **通信接口** |  |      |     |      |
| IIC     |   √  |  √   |  √   |  √  |
| **工作模式** |  |      |     |      |
| 轮询    |  √   | √     |  √   |   √  |
| 中断    |      |       |     |       |
| FIFO    |      |       |     |       |

## 使用说明

### 依赖

- RT-Thread 4.0.0+
- Sensor 组件
- libc 组件
- IIC 驱动：zmod4410 设备使用 IIC 进行数据通讯，需要系统 IIC 驱动支持

### 获取软件包

使用 zmod4410 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

```
RT-Thread online packages  --->
  peripheral libraries and drivers  --->
    sensors drivers  --->
            zmod4410: Gas Sensor Module ZMOD4410 driver library.
                    Version (latest)  --->
```

**Version**：软件包版本选择，默认选择最新版本。

### 使用软件包

#### 读取数据

- 通过导出的测试命令 `zmod_demo ` ，判断能否成功读取空气质量数据。在 demo 程序中设置了 RA6M4-CPK 开发板的 `USER INPUT` 按钮为退出键。运行效果如下：

```shell
msh >zmod_demo

 pin number : 0x0105 
msh >Evaluate measurements in a loop. Press any key to quit.

*********** Measurements ***********
 Rmox[0] = 0.100 kOhm
 Rmox[1] = 0.100 kOhm
 Rmox[2] = 0.100 kOhm
 Rmox[3] = 0.100 kOhm
 Rmox[4] = 0.100 kOhm
 Rmox[5] = 0.100 kOhm
 Rmox[6] = 0.100 kOhm
 Rmox[7] = 0.100 kOhm
 Rmox[8] = 0.100 kOhm
 Rmox[9] = 0.100 kOhm
 Rmox[10] = 0.100 kOhm
 Rmox[11] = 0.100 kOhm
 Rmox[12] = 0.100 kOhm
 log_Rcda = 0.000 logOhm
 EtOH =  0.008 ppm
 TVOC =  0.016 mg/m^3
 eCO2 =  400 ppm
 IAQ  =  1.0
Warmup!
************************************
```

## 注意事项

- 无

## 联系人信息

维护人:

- 维护：[Sherman](shaopengyu@rt-thread.com)
- 主页：https://github.com/ShermanShao/zmod4410
