# 总体架构

```
├── ARM/          # 内核相关层 
├── App/          # 应用逻辑层
├── FW/           # 固件层
├── HW/           # 硬件驱动层
└── Project/      # 工程管理层
```



APP应用逻辑层：

- Main.c：程序入口，初始化系统并启动主循环
- Heat.c/Motor.c/Audio.c：具体功能模块
- KeyOne.c/ProcKeyOne.c：按键逻辑处理，KeyOne判断长按、短按与双击逻辑，ProcKeyOne执行对应任务函数

HW硬件驱动层：

- ADC.c/PWM.c/UART1.c：模数转换、脉冲调制、串口通信
- Timer.c：定时器配置
- RCC.c：时钟配置
- Queue.c：用于串口数据缓冲队列

FW固件库层：包括产家提供的固件库函数

ARM内核相关层：包括启动文件、SysTick配置等

Project工程管理层：存放集成开发环境的项目配置文件，编译输出路径



依赖关系：

```
App -> HW -> FW -> ARM
```

Motor.c需要调用PWM.c的接口来调节转速。

Audio.c需要调用UART1.c来控制音频的播放。

Heat.c需要调用ADC.c实现温度检测。

驱动程序需要调用ARM和FW。

# 按摩模块



微型振动马达属于直流有刷电机，马达轴上面有一个偏心轮，当马达转动的时候，偏心轮的圆心质点不在电机的转心上，使得马达处于不断的失去平衡状态，由于惯性作用引起震动。



默认工作时间15min，状态机控制震动的频率/单次震动的时长，PWM频率为10kHz，PWM占空比控制强度

设定的震动模式有三种，通过按键切换：

- 模式一：强震，一直连续震动

- 模式二：脉冲，震动4s，停止1s

- 模式三：睡眠，渐强渐弱

# 主控芯片

`stm32f103rct6`

# 音频系统



```
音源（蓝牙/Flash语音）→ 功放 → 喇叭
```

音乐：控制音量、暂停、切歌

提示音：当按键切换时播报对应的本地录音

## 蓝牙

蓝牙模块+MCU：作为外设通过**串口**与单片机进行通信。

| 引脚及作用                                                   |
| ------------------------------------------------------------ |
| STATE：状态输出引脚。未连接时，则为低电平。连接成功时，则为高电平。 |
| RXD：串口接收引脚。接单片机的 TX 引脚。                      |
| TXD：串口发送引脚。接单片机的 RX 引脚。                      |
| GND：接地电源。                                              |
| VCC：输入 3.2～6V 的电源。                                   |
| KEY：按键按下进入AT模式。                                    |

`n8900u模块`

 <img src="C:\Users\yang'yu'wei\AppData\Roaming\Typora\typora-user-images\image-20260512161217238.png" alt="image-20260512161217238" style="zoom: 33%;" />

MCU串口控制协议：

`[起始码][指令类型][数据长度][数据1][数据2][...][数据n][校验和]`



<img src="C:\Users\yang'yu'wei\xwechat_files\wxid_ry4n3y3alnt022_9fed\temp\RWTemp\2026-05\5466a938dca6f8297e9365c795fa89da.jpg" alt="5466a938dca6f8297e9365c795fa89da" style="zoom: 25%;" /> 



0XF3 切换到flash模式

# 加热模块

热敷温度一般在40℃-50℃
低温：38°C
高温：42°C

NTC热敏电阻实现**温度检测**

```c
```



过热保护，温度控制

```c
void SetTemp(u8 temp)
{
  u8 nowTemp = DetectTemp();
  if((nowTemp < temp - HYSTTEMP) && (nowTemp < HIGHTEMP)){   //当前温度小于设定温度
    HeatOn();                           //开启加热
  }
  else if(nowTemp > temp + HYSTTEMP){
    HeatOff();                          //关闭加热
  }
}
```



# 按键

长按：长按2秒进行开/关机

短按：短按进行模式切换



# 电源管理

## 电池电量检测

- 开路电压检测：

$$
V_{bat}=\frac{ADC}{4096} \cdot V_{ref} \cdot K
$$

ADC：采样值，Vref：参考电压，K：分压系数

```c
float readBattery()
{
  float voltage;
  u16 value;
  
  ReadADCBuf(&value);
  voltage=(float)value*3.3/4096*2;
  
  return voltage;
}
```

- 将电压值映射为百分比：锂电池满电约4.2V，放电终止约3.0V。开路电压与电池容量的对应关系呈非线性。

<img src="C:\Users\yang'yu'wei\AppData\Roaming\Typora\typora-user-images\image-20260423171448548.png" alt="image-20260423171448548" style="zoom: 33%;" /> 

法1：线性映射 $$SOC=\frac{V-V_{min}}{V_{max}-V_{min}}\times 100\%$$

法2：分段线性映射

法3：查表法

```
BatteryCurve table[] = {
    {4.20, 100},
    {4.06, 90},
    {3.98, 80},
    {3.92, 70},
    {3.87, 60},
    {3.82, 50},
    {3.79, 40},
    {3.77, 30},
    {3.74, 20},
    {3.68, 10},
    {3.45, 5},
    {3.00, 0}
};
```
