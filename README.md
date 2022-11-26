# stm32-sine-wave-outputing-by-PWM-(proteus-simulating)
Proteus8仿真 stm32f401VE实现PWM输出正弦波、方波、三角波，且频率在1-100可调 Keil5编写代码

 题目

![image](https://user-images.githubusercontent.com/61747149/194799274-8cd1746e-95de-4027-a838-27941f98b541.png)

 运行图

![image](https://user-images.githubusercontent.com/61747149/194800233-928ad00a-2e50-4986-8c46-d2ba6ed069d3.png)

### 操作说明：
仿真中

MODE为模式键，模式显示在最左边数码管。

0-5共六种模式中，0-2分别表示PB0口输出正弦波，三角波、锯齿波，3-5表示PB1口输出正弦波，三角波、锯齿波

F代表输出波形的频率，在液晶屏右边三个数码管显示，有F-1，F-5，F-10三种选择，减到0直接变回100

调好模式和频率后，按下ENABLE键，则输出模式对应频率的信号。

此仿真较占内存，运行时间步长小，数码管显示不连续是正常现象。


### 基本原理
stm32引脚输出PWM波，PWM波形不同的占空比，有对应的不同直流分量，通过简单的RC滤波，将直流分量提取出来，就可以通过控制PWM输出占空比来控制输出电压，从而输出各种波形。

### 实现方法
程序使用了TIM2、3、4、5其中TIM3向PB0和PB1输出PWM波，TIM4负责为LED刷新定时，TIM2和TIM5分别控制PB0和PB1输出信号的频率。五个按键KEY1、2、3、4、5作用分别是选择信号的模式、频率-1、使能输出、频率-5、频率-10。初始化相关设置之后，可以输出两路100Hz的正弦波，当按键设置更改模式和频率后，重新初始化TIM2、5到相应的信号频率，并更改输出到相应的信号模式，完成两路信号的变频输出。
