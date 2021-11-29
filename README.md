# ZMOD4410

## 简介

zmod4410 软件包使用了空气质量传感器 `zmod4410` 的基本功能。

传感器 `zmod4410` 的输入电压为 `1.7V ~ 3.6V` 范围内，[zmod4410 详细功能参数介绍](https://www2.renesas.cn/cn/zh/products/sensor-products/environmental-sensors/digital-gas-sensors/zmod4410-indoor-air-quality-sensor-platform
)

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

zmod4410 的初始化函数如下所示：
```c
int rt_hw_zmod4410_init(const char *name, struct rt_sensor_config *cfg);
```

该函数需要由用户调用，函数主要完成的功能有，

- 设备配置和初始化（根据传入的配置信息配置接口设备）；
- 注册相应的传感器设备，完成 zmod4410 传感器设备的注册；

#### 初始化示例
```c
#include "sensor_renesas_zmod4410.h"
#define ZMOD4410_I2C_BUS "i2c1"
int rt_hw_zmod4410_port(void)
{
    struct rt_sensor_config cfg;
    cfg.intf.dev_name  = ZMOD4410_I2C_BUS;
    rt_hw_zmod4410_init("zmod4410", &cfg);
    return RT_EOK;
}
INIT_ENV_EXPORT(rt_hw_zmod4410_port);

```
#### 读取数据

- 数据已接入 rt-thread 传感器框架，可以使用 `sensor` 相关命令读取传感器信息
  - 测试命令 `sensor_polling iaq_zmod` ，验证能否读取 IAQ 数据。
  - 测试命令 `sensor_polling tvoc_zmo` ，验证能否读取 TVOC 数据。
  - 测试命令 `sensor_polling etoh_zmo` ，验证能否读取 EtOH 数据。
  - 测试命令 `sensor_polling eco2_zmo` ，验证能否读取 eCO2 数据。

运行效果如下：

```shell
msh />sensor_polling iaq_zmod
[309438] I/sensor.zmod4410: Warmup!
[311432] I/sensor.cmd: num:  0, IAQ:    1.0 , timestamp:946684800
[311740] I/sensor.zmod4410: Warmup!
[313735] I/sensor.cmd: num:  1, IAQ:    1.0 , timestamp:946684800
[314043] I/sensor.zmod4410: Warmup!
[316038] I/sensor.cmd: num:  2, IAQ:    1.0 , timestamp:946684800
[316346] I/sensor.zmod4410: Warmup!
[318341] I/sensor.cmd: num:  3, IAQ:    1.0 , timestamp:946684800
[318649] I/sensor.zmod4410: Warmup!
[320644] I/sensor.cmd: num:  4, IAQ:    1.0 , timestamp:946684800
[320952] I/sensor.zmod4410: Warmup!
[322947] I/sensor.cmd: num:  5, IAQ:    1.0 , timestamp:946684800
[323255] I/sensor.zmod4410: Warmup!
```

- 也可以通过 `demo.c` 中导出的测试命令 `zmod_demo` ，判断能否成功读取空气质量数据。在 demo 程序中设置了 RA6M4-CPK 开发板的 `USER INPUT` 按钮为退出键。运行效果如下：
> 注意：`demo.c` 和 `sensor_renesas_zmod4410.c` 只能有一个被添加到工程中参与编译。


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
