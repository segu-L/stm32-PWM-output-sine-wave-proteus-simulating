#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
//GUET 1900200920 LSH
//选择R=1k	C=0.1uF		波形抖动为0.07V		锯齿波陡降变化需要约0.2ms
//KEY1为模式选择	KEY2为频率-1	KEY3为使能当前模式和频率	KEY4为频率-5	KEY5为频率10
//Mode取值0-5，其中0-2代表PB0输出的三个模式	3-5代表PB1输出的三个模式		数码管低三位显示对应频率，第四位显示模式（当显示9999为报错）
//TIM2为PB0输出频率定时器	TIM3为两路PWM波输出		TIM4为显示屏输出		TIM5为PB1输出频率定时器
//Index为当前时刻取对应模式的信号数组的序号
int Mode1=0,Mode2=3;	//Mode1,Mode2为模式(确定)
int Mode=0,F=100,Index1=0,Index2=0;	//mode为模式(选择) 1/3:正弦波 2/4:三角波 3/5:锯齿波		F为波的频率（1-100）	Index为当前时刻取数组的序号（0-99）
static unsigned char sin[ ]      ={53,56,59,62,65,68,71,73,76,79,81,83,86,88,90,91,93,94,96,97,97,98,99,99,99,99,99,98,97,97,96,94,93,91,90,88,86,83,81,79,76,73,71,68,65,62,59,56,53,50,46,43,40,37,34,31,28,26,23,20,18,16,13,11,9,8,6,5,3,2,2,1,0,0,0,0,0,1,2,2,3,5,6,8,9,11,13,16,18,20,23,26,28,31,34,37,40,43,46,49};
static unsigned char delta[ ]    ={0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95,97,99,99,97,95,93,91,89,87,85,83,81,79,77,75,73,71,69,67,65,63,61,59,57,55,53,51,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0};
static unsigned char sawtooth[ ] ={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99};
static unsigned char LEDSEG[ ] ={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
static unsigned char LEDDIG[ ] = {0x0E,0x0D,0x0B,0x07};
static unsigned char LEDBuffer [4]= { 0,0,1,0};
static unsigned char LEDIndex;

void LEDPin_Init(void)
{
	GPIO_InitTypeDef MyGPIO;

	RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOEEN,ENABLE);

	MyGPIO.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1|GPIO_Pin_2 | GPIO_Pin_3 |GPIO_Pin_4 |GPIO_Pin_5|GPIO_Pin_6 | GPIO_Pin_7 |GPIO_Pin_9 | GPIO_Pin_10| GPIO_Pin_11 |GPIO_Pin_12;
	MyGPIO.GPIO_Mode = GPIO_Mode_OUT;
	MyGPIO.GPIO_OType =GPIO_OType_PP;
	MyGPIO.GPIO_Speed = GPIO_Speed_2MHz;
	MyGPIO.GPIO_PuPd= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE,&MyGPIO);
}

void KeyPin_Init (void)
{
	GPIO_InitTypeDef MyGPIO;
	RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIODEN,ENABLE);
	MyGPIO.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_6 |GPIO_Pin_7| GPIO_Pin_8 | GPIO_Pin_9;
	MyGPIO.GPIO_Mode = GPIO_Mode_IN;
	MyGPIO.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init (GPIOD,&MyGPIO) ;
}

void Key_EXTI_Init (void)
{
	EXTI_InitTypeDef MyEXTI;
	
	RCC_APB2PeriphClockCmd (RCC_APB2ENR_SYSCFGEN,ENABLE);
	
	RCC_APB2PeriphClockCmd (RCC_APB2ENR_SYSCFGEN,ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD,EXTI_PinSource5) ;
	MyEXTI.EXTI_Line = EXTI_Line5;
	MyEXTI.EXTI_Mode = EXTI_Mode_Interrupt;
	MyEXTI.EXTI_Trigger = EXTI_Trigger_Falling;
	MyEXTI.EXTI_LineCmd = ENABLE;
	EXTI_Init (&MyEXTI) ;
	
	RCC_APB2PeriphClockCmd (RCC_APB2ENR_SYSCFGEN,ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD,EXTI_PinSource6) ;
	MyEXTI.EXTI_Line = EXTI_Line6;
	MyEXTI.EXTI_Mode = EXTI_Mode_Interrupt;
	MyEXTI.EXTI_Trigger = EXTI_Trigger_Falling;
	MyEXTI.EXTI_LineCmd = ENABLE;
	EXTI_Init (&MyEXTI) ;
	
	RCC_APB2PeriphClockCmd (RCC_APB2ENR_SYSCFGEN,ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD,EXTI_PinSource7) ;
	MyEXTI.EXTI_Line = EXTI_Line7;
	MyEXTI.EXTI_Mode = EXTI_Mode_Interrupt;
	MyEXTI.EXTI_Trigger = EXTI_Trigger_Falling;
	MyEXTI.EXTI_LineCmd = ENABLE;
	EXTI_Init (&MyEXTI) ;
	
	RCC_APB2PeriphClockCmd (RCC_APB2ENR_SYSCFGEN,ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD,EXTI_PinSource8) ;
	MyEXTI.EXTI_Line = EXTI_Line8;
	MyEXTI.EXTI_Mode = EXTI_Mode_Interrupt;
	MyEXTI.EXTI_Trigger = EXTI_Trigger_Falling;
	MyEXTI.EXTI_LineCmd = ENABLE;
	EXTI_Init (&MyEXTI) ;
	
	RCC_APB2PeriphClockCmd (RCC_APB2ENR_SYSCFGEN,ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD,EXTI_PinSource9) ;
	MyEXTI.EXTI_Line = EXTI_Line9;
	MyEXTI.EXTI_Mode = EXTI_Mode_Interrupt;
	MyEXTI.EXTI_Trigger = EXTI_Trigger_Falling;
	MyEXTI.EXTI_LineCmd = ENABLE;
	EXTI_Init (&MyEXTI) ;
}

void all_NVIC_Init (void)
{
	NVIC_InitTypeDef MyNVIC;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	MyNVIC.NVIC_IRQChannel =TIM3_IRQn;
	MyNVIC.NVIC_IRQChannelPreemptionPriority = 2;
	MyNVIC.NVIC_IRQChannelSubPriority = 2;
	MyNVIC.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &MyNVIC);
	
	MyNVIC.NVIC_IRQChannel =TIM2_IRQn;
	MyNVIC.NVIC_IRQChannelPreemptionPriority = 2;
	MyNVIC.NVIC_IRQChannelSubPriority = 1;
	MyNVIC.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &MyNVIC);
	
	MyNVIC.NVIC_IRQChannel =TIM5_IRQn;
	MyNVIC.NVIC_IRQChannelPreemptionPriority = 2;
	MyNVIC.NVIC_IRQChannelSubPriority = 1;
	MyNVIC.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&MyNVIC);
	
	MyNVIC.NVIC_IRQChannel =EXTI9_5_IRQn ;
	MyNVIC.NVIC_IRQChannelPreemptionPriority = 0;
	MyNVIC.NVIC_IRQChannelSubPriority = 0;
	MyNVIC.NVIC_IRQChannelCmd =ENABLE;
	NVIC_Init (&MyNVIC);
	
	MyNVIC.NVIC_IRQChannel =TIM4_IRQn;
	MyNVIC.NVIC_IRQChannelPreemptionPriority = 2;
	MyNVIC.NVIC_IRQChannelSubPriority = 3;
	MyNVIC.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&MyNVIC);
}

void TIM3_OC3_Config_PWM(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE); 
	// 使能TIM3时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource0,GPIO_AF_TIM3); //TIM3输出通道3引脚复用 
	/* TIM3输出通道3引脚配置 */															   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	TIM_TimeBaseStructure.TIM_Period = 100-1;       

	TIM_TimeBaseStructure.TIM_Prescaler = 1;	
	// 采样时钟分频
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	// 计数方式
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
		
	/*设置比较输出通道，PWM模式配置*/
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;	    //配置为PWM模式1,当定时器计数值小于CCR1_Val时 输出有效电平
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	
	TIM_OCInitStructure.TIM_Pulse = 90;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  	  //输出有效电平为高电平
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);	 //使能通道1  
	/*使能通道3重载*/
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
	 	
	// 使能定时器
	TIM_Cmd(TIM3, ENABLE);	

}

void TIM3_OC4_Config_PWM(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE); 
	// 使能TIM3时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource1,GPIO_AF_TIM3); //TIM3输出通道3引脚复用 															   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	TIM_TimeBaseStructure.TIM_Period = 100 -1;       

	TIM_TimeBaseStructure.TIM_Prescaler = 1;	
	// 采样时钟分频
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	// 计数方式
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;	    //配置为PWM模式1,当定时器计数值小于CCR1_Val时 输出有效电平
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	
	TIM_OCInitStructure.TIM_Pulse = 30;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  	  //输出有效电平为高电平
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);	 //使能通道1  
	/*使能通道3重载*/
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_Cmd(TIM3, ENABLE);		
}
//控制PB0波形的频率，输入信号频率f，设置相对应的计数周期，使1/f秒一次中断
void TIM2_Init (int f)
{
	TIM_TimeBaseInitTypeDef MyTIM;
	RCC_APB1PeriphClockCmd (RCC_APB1ENR_TIM2EN,ENABLE);
	MyTIM.TIM_Prescaler = 16-1;
	MyTIM.TIM_Period =(int)(10000/f)-1;
	MyTIM.TIM_CounterMode = TIM_CounterMode_Up;
	MyTIM.TIM_ClockDivision = 0;
	TIM_TimeBaseInit (TIM2,&MyTIM);
	TIM_ITConfig(TIM2,TIM_IT_Update, ENABLE);
	TIM_Cmd (TIM2,ENABLE);
}

//控制PB1波形的频率，输入信号频率f，设置相对应的计数周期，使1/f秒一次中断
void TIM5_Init (int f)
{
	TIM_TimeBaseInitTypeDef MyTIM;
	RCC_APB1PeriphClockCmd (RCC_APB1ENR_TIM5EN,ENABLE);
	MyTIM.TIM_Prescaler = 16-1;
	MyTIM.TIM_Period =(int)(10000/f)-1;
	MyTIM.TIM_CounterMode = TIM_CounterMode_Up;
	MyTIM.TIM_ClockDivision = 0;
	TIM_TimeBaseInit (TIM5,&MyTIM);
	TIM_ITConfig(TIM5,TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM5,ENABLE);
}

//数码管显示，1ms一次中断	低三位显示要选择的频率，第四位显示要选择的模式
void TIM4_Init (void)
{
	TIM_TimeBaseInitTypeDef MyTIM;
	RCC_APB1PeriphClockCmd (RCC_APB1ENR_TIM4EN,ENABLE);
	MyTIM.TIM_Prescaler = 16-1;
	MyTIM.TIM_Period =1000-1;
	MyTIM.TIM_CounterMode = TIM_CounterMode_Up;
	MyTIM.TIM_ClockDivision = 0;
	TIM_TimeBaseInit (TIM4,&MyTIM);
	TIM_ITConfig(TIM4,TIM_IT_Update, ENABLE);
	TIM_Cmd (TIM4,ENABLE);
}

//数码管显示，1ms一次中断
void TIM4_IRQHandler (void)
{
	if(RESET !=TIM_GetITStatus(TIM4,TIM_IT_Update) )
	{
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update) ;
		
		GPIO_Write (GPIOE,(LEDDIG[LEDIndex] <<9) |(LEDSEG[LEDBuffer[LEDIndex]]));
		if(++LEDIndex >= sizeof(LEDBuffer))
			LEDIndex = 0 ;
		
	}
}
//控制PB0波形的频率，100Hz时0.1ms一次中断
void TIM2_IRQHandler (void)
{
	if(RESET !=TIM_GetITStatus(TIM2,TIM_IT_Update) )
	{
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
		if(++Index1==100)	Index1=0;
		if(Mode1==0)
			TIM_SetCompare3(TIM3,sin[Index1]);
		else if(Mode1==1)
			TIM_SetCompare3(TIM3,delta[Index1]);
		else if(Mode1==2)
			TIM_SetCompare3(TIM3,sawtooth[Index1]);
		else
		{
			LEDBuffer [0]= 9;
			LEDBuffer [1]= 9;
			LEDBuffer [2]= 9;
			LEDBuffer [3]= 9;
		}
	}
}

//控制PB1波形的频率，100Hz时0.1ms一次中断
void TIM5_IRQHandler (void)
{
	if(RESET !=TIM_GetITStatus(TIM5,TIM_IT_Update) )
	{
		TIM_ClearITPendingBit(TIM5,TIM_IT_Update);
		if(++Index2==100)	Index2=0;
		if(Mode2==3)
			TIM_SetCompare4(TIM3,sin[Index2]);
		else if(Mode2==4)
			TIM_SetCompare4(TIM3,delta[Index2]);
		else if(Mode2==5)
			TIM_SetCompare4(TIM3,sawtooth[Index2]);
		else
		{
			LEDBuffer [0]= 9;
			LEDBuffer [1]= 9;
			LEDBuffer [2]= 9;
			LEDBuffer [3]= 9;
		}
		
	}
}

void EXTI9_5_IRQHandler (void)
{
	
	//按键PD5，调节信号模式
	if(RESET !=EXTI_GetITStatus (EXTI_Line5) )
	{
		EXTI_ClearITPendingBit(EXTI_Line5) ;
		if(++Mode==6)	
			Mode=0;
		LEDBuffer[3]= Mode;
	}
	
	//按键PD6，调节信号频率-1
	if(RESET !=EXTI_GetITStatus (EXTI_Line6) )
	{
		EXTI_ClearITPendingBit(EXTI_Line6) ;
		if(--F<1)	
			F=100;
		LEDBuffer [0]= F%10;
		LEDBuffer [1]= (F/10)%10;
		LEDBuffer [2]= (F/100)%10;
	}
	//按键PD7，模式Mode和频率F使能
	if(RESET !=EXTI_GetITStatus (EXTI_Line7) )
	{
		EXTI_ClearITPendingBit(EXTI_Line7) ;
		if(Mode<3){	Mode1=Mode;		TIM2_Init (F);}
		if(Mode>2){	Mode2=Mode;		TIM5_Init (F);}
	}
	//按键PD8，调节信号频率-5
	if(RESET !=EXTI_GetITStatus (EXTI_Line8) )
	{
		EXTI_ClearITPendingBit(EXTI_Line8) ;
		F-=5;
		if(F<1)	
			F=100;
		LEDBuffer [0]= F%10;
		LEDBuffer [1]= (F/10)%10;
		LEDBuffer [2]= (F/100)%10;
	}
	//按键PD9，调节信号频率-10
	if(RESET !=EXTI_GetITStatus (EXTI_Line9) )
	{
		EXTI_ClearITPendingBit(EXTI_Line9) ;
		F-=10;
		if(F<1)	
			F=100;
		LEDBuffer [0]= F%10;
		LEDBuffer [1]= (F/10)%10;
		LEDBuffer [2]= (F/100)%10;
	}
	
}


int main (void)
{
	LEDPin_Init();
	KeyPin_Init();
	Key_EXTI_Init();
	TIM4_Init();
	TIM3_OC3_Config_PWM();
	TIM3_OC4_Config_PWM();
	all_NVIC_Init();
	TIM2_Init(100);
	TIM5_Init(100);
	
	while(1)
	{}
}



