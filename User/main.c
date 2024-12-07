#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Timer.h"
#include <math.h>
/*
接口引脚定义 PA0123———pwm通道1234
软件模拟iic的OLED显示屏幕 PB89 SCL SDA
旋转编码器 PA7 PA6接口 可自由调整方向
4个按钮按键接GND和（pb0 pb1 pa3 pa5）
分别对应加 减 确定 返回
实测型号STM32F103RCT6 
代码体量不大，理论兼容各种带4定时器的stm32f103型号
*/
unsigned char Key_Slow_Down = 0;
unsigned char Key_Val,Key_Old,Key_Up,Key_Down;
unsigned char display_mode = 0;
unsigned char choose_flag = 2;
unsigned char pwm_display = 0;
unsigned char pwmch = 0;
unsigned char ccr1,ccr2,ccr3,ccr4;
unsigned int freq = 0;
unsigned char duty1,duty2,duty3,duty4;
unsigned char flash_flag = 0;
unsigned int flash_time = 0;
unsigned char sec = 0;
unsigned char min = 0;
unsigned char hour = 0;
unsigned time1s = 0;
unsigned char OK_flag = 0;
unsigned int psc;
unsigned int set_psc;
unsigned char set_ccr1 = 0;
unsigned char set_ccr2 = 0;
unsigned char set_ccr3 = 0;
unsigned char set_ccr4 = 0;
unsigned int delay1000ms = 0;
unsigned char other_flag = 1;
unsigned char sinsetup[4] = {0,0,0,0};//针对sin四个通道的启动情况描述
unsigned char cossetup[4] = {0,0,0,0};
unsigned char tansetup[4] = {0,0,0,0};
unsigned char lnesetup[4] = {0,0,0,0};
unsigned char trisetup[4] = {0,0,0,0};
unsigned char sawsetup[4] = {0,0,0,0};
//ch1 ch2 ch3 ch4 标志位用于对other模式的启动与关闭
//同时ch1 ch2 ch3 ch4的值也可反映对应通道的模式
unsigned char ch1 = 0;
unsigned char ch2 = 0;
unsigned char ch3 = 0;
unsigned char ch4 = 0;
unsigned char other_choose_flag = 0;
unsigned char sin0,cos0,tan0;
unsigned char delay100ms = 0;
unsigned char  sin1,cos1,tan1,tank;
unsigned char lne1,tri1,saw1;
//定义为其他标志位，用于在displaymode4 OK_flag=1的情况下选择

//函数声明
void setting_display();
double sin (double x);
double cos (double x);
double tan (double x);
void Other_mode(void);

/*分隔符*/
/*相关资源分配
定时器1；TIM1/尚未启用
定时器2：实现四通道pwm波形生成
定时器3：TIM3/尚未启用
定时器4：程序1毫秒中断
外部中断：用于处理旋转编码器所产生的信号

*/

void pwm_init(void)    //pa3暂时不可用，这里仅设计ch1 2 3通道 对应pa012；
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//开启TIM2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;     //配置为复用推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							//将PA011引脚初始化为复用推挽输出	
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM2);		//选择TIM2为内部时钟，若不调用此函数，TIM默认也为内部时钟
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;				//计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;				//预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM2的时基单元
	/*输出比较初始化*/ //oc Out Compare
	TIM_OCInitTypeDef TIM_OCInitStructure;							//定义结构体变量
	TIM_OCStructInit(&TIM_OCInitStructure);      //结构体初始化，若结构体没有完整赋值,避免结构体不完整导致的错误
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               //输出比较模式，选择PWM模式1
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       //输出极性，选择为高，若选择极性为低，则输出高低电平取反
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   //输出使能
	TIM_OCInitStructure.TIM_Pulse = 0;								//初始的CCR值
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);                        //将结构体变量交给TIM_OC2Init，配置TIM2的输出比较通道2
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);                        //对四个通道都进行配置
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);
	/*TIM使能*/
	TIM_Cmd(TIM2, ENABLE);			//使能TIM2，定时器开始运行
}

void Encoder_Init(void)   //旋转编码器 pa6 pa7引脚作用
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOB的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);		//开启AFIO的时钟，外部中断必须开启AFIO的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);						//将PB0和PB1引脚初始化为上拉输入
	
	/*AFIO选择中断引脚*/
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);//将外部中断的0号线映射到GPIOB，即选择PB0为外部中断引脚
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);//将外部中断的1号线映射到GPIOB，即选择PB1为外部中断引脚
	
	/*EXTI初始化*/
	EXTI_InitTypeDef EXTI_InitStructure;						//定义结构体变量
	EXTI_InitStructure.EXTI_Line = EXTI_Line6 | EXTI_Line7;		//选择配置外部中断的0号线和1号线
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//指定外部中断线使能
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//指定外部中断线为中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//指定外部中断线为下降沿触发
	EXTI_Init(&EXTI_InitStructure);								//将结构体变量交给EXTI_Init，配置EXTI外设
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				//配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环之前
																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//选择配置NVIC的EXTI9-5线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//指定NVIC线路的抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设

}



void start_display()
{
	unsigned char i;
	OLED_ShowString(2,1,"     Welcome    ");
	OLED_ShowString(3,1,"   Loading...   ");
	for(i=0;i<101;i++)
		{OLED_ShowNum(4,14,i,3);
		Delay_ms(8);
		}
}	
void keyinit()      //按键初始化函数  按键pb0 pb1 pa3 pa5
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef gpioa1;
	gpioa1.GPIO_Mode = GPIO_Mode_IPU;
	gpioa1.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	gpioa1.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&gpioa1);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef gpiob1;
	gpiob1.GPIO_Mode = GPIO_Mode_IPU;
	gpiob1.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	gpiob1.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpiob1);
}

unsigned char keyread()
{ 
  if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0)==0) {return 1;}
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)==0) {return 2;}
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3)==0) {return 3;}
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)==0) {return 4;}
	else return 0;
}
void Key1op(void)
{
			OLED_Clear();
			
			if(display_mode==3&&OK_flag==0)      //设置模式  //当且仅当未处于设置模式且显示模式为3时有效
			{choose_flag++;
			if(choose_flag==7) choose_flag=6;}
			
			if(display_mode==4&&OK_flag==0)  //在其他波形选择界面未切换
			{
			choose_flag++;
			if(choose_flag==7)choose_flag=6;
			}

			if(display_mode==3&&OK_flag==1)
			{
			switch(choose_flag)    //chooseflag = ccr+1
			{
				case 1: psc++;break;
				case 2: ccr1++;break;
				case 3: ccr2++;break;
				case 4: ccr3++;break;
				case 5: ccr4++;break;
			}
			}	
			
			if(display_mode==0)
			{choose_flag++;
				if(choose_flag==6)choose_flag=5;}   //上限为5
			
			if(display_mode==1)
			{choose_flag++;
			if(choose_flag==5)choose_flag=4;}	
			
			if(display_mode==4&&OK_flag==1)
			{
			other_flag++;
			if(other_flag==5)other_flag=4;
			}
}

void Key2op(void)
{
OLED_Clear();	
			if(display_mode==0)
			{choose_flag--;
			if(choose_flag==1)choose_flag=2;}
			if(display_mode==4&&OK_flag==0)
			{choose_flag--;
			if(choose_flag==0)
				choose_flag=1;
			}
			if(display_mode==1)
			{choose_flag--;
			if(choose_flag==0)choose_flag=1;}
			
			if(display_mode==4&&OK_flag==1)
			{
				other_flag--;
				if(other_flag==0)other_flag=1;
				
			}
			
			if(display_mode==3&&OK_flag==0)
			{choose_flag--;
			if(choose_flag==0)choose_flag=1;}
			
			if(display_mode==3&&OK_flag==1)
			{
			switch(choose_flag)    //chooseflag = ccr+1
			{
				case 1: psc--;break;
				case 2: ccr1--;break;
				case 3: ccr2--;break;
				case 4: ccr3--;break;
				case 5: ccr4--;break;
			}}
}

void Get_CCR(void)
{
		set_ccr1 = TIM_GetCapture1(TIM2);
		set_ccr2 = TIM_GetCapture2(TIM2);
		set_ccr3 = TIM_GetCapture3(TIM2);
		set_ccr4 = TIM_GetCapture4(TIM2);
}
void Key_Proc()
		{
		if (Key_Slow_Down) return;
		Key_Slow_Down = 1;
		Key_Val = keyread();
		Key_Down = Key_Val & (Key_Old ^ Key_Val);
		Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
		Key_Old = Key_Val;
			

			switch (Key_Down)
			{
			case 1:  //按键1，选项加加
      Key1op();
			break;
	    
			case 2:      //按键2，选项减减       //设置模式下，当且仅当未处于设置模式且显示模式为3时有效
			Key2op();
			break;    

			case 3:      //displaymode=0时为ok按键 界面切换中切记OLED_Clear
			OLED_Clear();
			if(display_mode==0)
				{
						switch(choose_flag)
						{ case 2:display_mode=1;choose_flag = 1;break;
							case 3:display_mode=2;choose_flag = 1;break;
							case 4:display_mode=3;//向着setting切换过程中
							//从系统寄存器中读取各个通道ccr的值
							set_ccr1 = TIM_GetCapture1(TIM2);
							set_ccr2 = TIM_GetCapture2(TIM2);
							set_ccr3 = TIM_GetCapture3(TIM2);
							set_ccr4 = TIM_GetCapture4(TIM2);
							set_psc = TIM_GetPrescaler(TIM2);
							//将读取的值放入内部变量setccr,随即赋值给显示ccr,
							ccr1 = set_ccr1;ccr2 = set_ccr2;
							ccr3 = set_ccr3;ccr4 = set_ccr4;
							psc = set_psc;
							choose_flag = 1;break;
							case 5:OLED_Clear();display_mode=4;OLED_Clear();
							choose_flag=1;break;  //从主界面向other界面切换 
						}
						break;
			  }			
			
				if(display_mode==1)               //pwm直接显示/调整
			{ 
				
				pwmch = choose_flag;
				pwm_display = 1;                  //开启pwm显示 并获取实时pwm对应参数的值
				set_ccr1 = TIM_GetCapture1(TIM2);
				set_ccr2 = TIM_GetCapture2(TIM2);
				set_ccr3 = TIM_GetCapture3(TIM2);
				set_ccr4 = TIM_GetCapture4(TIM2);
				set_psc = TIM_GetPrescaler(TIM2);
				freq = 720000 / (set_psc + 1);
				break;
			}
			//针对波形模式进行选中，然后把它赋值给标志位
			if(display_mode==4&&OK_flag==0){ 
			OK_flag=1;
			other_choose_flag = choose_flag;
			OLED_Clear();break;}
			
			if(display_mode==4&&OK_flag==1)  //对选中项目对应的标志位进行赋值
			{
				switch(other_flag)  //取反操作  otherflag为对ch通道选择
					{
						/*假设这里选sin模式并进入
						other_choose_flag会被赋值为1
						*/
						case 1:
							if(ch1!=0){ch1=0;break;}
							if(ch1==0){ch1=other_choose_flag;break;}
							//该操作可以直接将模式赋值给通道号
							
							break;
						case 2:
							if(ch2!=0){ch2=0;break;}
							if(ch2==0){ch2=other_choose_flag;break;}
							break;
						case 3:
							if(ch3!=0){ch3=0;break;}
							if(ch3==0){ch3=other_choose_flag;break;}
							break;
						case 4:
							if(ch4!=0){ch4=0;break;}
							if(ch4==0){ch4=other_choose_flag;break;}
							break;
					}
					break;
			
			}
			
			if(display_mode==3&&OK_flag==0)   //确定逻辑
			{OLED_Clear();OK_flag=1;flash_flag=1;break;}
			
			if(display_mode==3&&OK_flag==1)   //此时按下为保存逻辑
			{
				set_ccr1 = ccr1;set_ccr2 = ccr2;
				set_ccr3 = ccr3;set_ccr4 = ccr4;
				set_psc = psc;
				TIM_SetCompare1(TIM2,set_ccr1);
				TIM_SetCompare2(TIM2,set_ccr2);
				TIM_SetCompare3(TIM2,set_ccr3);
				TIM_SetCompare4(TIM2,set_ccr4);
				TIM_PrescalerConfig(TIM2,set_psc,TIM_PSCReloadMode_Immediate);
				OK_flag=0;break;
			}
			
			break;
						case 4:     //按钮4为返回按钮
				OLED_Clear();
			//以下全为退出逻辑 不涉及保存或更新
			if(display_mode==1&&pwm_display==1){pwm_display=0;break;}
			if(display_mode==1&&pwm_display==0){display_mode=0;Get_CCR();choose_flag=2;break;}
			if(display_mode==3&&OK_flag==0){display_mode=0;Get_CCR();choose_flag=2;break;}
			if(display_mode==3&&OK_flag==1){OK_flag=0;flash_flag=0;break;}
			if(display_mode==2){display_mode = 0;choose_flag=2;break;}
			if(display_mode==4&&OK_flag==0){display_mode=0;Get_CCR();choose_flag=2;break;}  //从其他界面跳转到主菜单
			if(display_mode==4&&OK_flag==1){OK_flag=0;other_choose_flag=0;break;}
			
			break;
			}

      }  //if的
			

//其他波形函数扫描程序，将ch中的值赋值给对应数组
void scan_channel(void)
{
	switch(ch1)
	{
		case 0:sinsetup[0] = 0;
			     cossetup[0] = 0;
		       tansetup[0] = 0;
		       lnesetup[0] = 0;
		       trisetup[0] = 0;
		       sawsetup[0] = 0;break;
		case 1:sinsetup[0] = 1;break;
		case 2:cossetup[0] = 1;break;
		case 3:tansetup[0] = 1;break;
		case 4:lnesetup[0] = 1;break;
		case 5:trisetup[0] = 1;break;
		case 6:sawsetup[0] = 1;break;
	}
	switch(ch2)
	{	case 0:sinsetup[1] = 0;
			     cossetup[1] = 0;
		       tansetup[1] = 0;
		       lnesetup[1] = 0;
		       trisetup[1] = 0;
		       sawsetup[1] = 0;break;
		case 1:sinsetup[1] = 1;break;
		case 2:cossetup[1] = 1;break;
		case 3:tansetup[1] = 1;break;
		case 4:lnesetup[1] = 1;break;
		case 5:trisetup[1] = 1;break;
		case 6:sawsetup[1] = 1;break;

	}
	switch(ch3)
	{	case 0:sinsetup[2] = 0;
			     cossetup[2] = 0;
		       tansetup[2] = 0;
		       lnesetup[2] = 0;
		       trisetup[2] = 0;
		       sawsetup[2] = 0;break;
		case 1:sinsetup[2] = 1;break;
		case 2:cossetup[2] = 1;break;
		case 3:tansetup[2] = 1;break;
		case 4:lnesetup[2] = 1;break;
		case 5:trisetup[2] = 1;break;
		case 6:sawsetup[2] = 1;break;

					
	}
	switch(ch4)
	{	case 0:sinsetup[3] = 0;
			     cossetup[3] = 0;
		       tansetup[3] = 0;
		       lnesetup[3] = 0;
		       trisetup[3] = 0;
		       sawsetup[3] = 0;break;
		case 1:sinsetup[3] = 1;break;
		case 2:cossetup[3] = 1;break;
		case 3:tansetup[3] = 1;break;
		case 4:lnesetup[3] = 1;break;
		case 5:trisetup[3] = 1;break;
		case 6:sawsetup[3] = 1;break;

					
	}
	
}
//将ch中的值赋值给对应数组,数组实现波形模式的通断和启动
//一个数组4个数据 分别表示ch1234通道
void OLED_display(void)
{
  //模式0为菜单选择模式
	switch(display_mode)
	{ 
		case 0:    
				/*choose flag 与选择项互相对应 */
		OLED_ShowString(1,1,"    MainMenu");  //主菜单 长亮显示
		if(choose_flag<5)
		{
		OLED_ShowString(2,2,"PWM Set");           //pwm配置
		OLED_ShowString(3,2,"Clock Test");             //时钟
		OLED_ShowString(4,2,"Setting");           //设置
		OLED_ShowString(choose_flag,1,">");
		}
		if(choose_flag==5)  //选择第五项
		{
		OLED_ShowString(4,1,">");
		OLED_ShowString(2,2,"Clock Test");             //时钟
		OLED_ShowString(3,2,"Setting");           //设置
		OLED_ShowString(4,2,"Other_Mode");        //其他模式          
		}
		if(set_ccr1!=0)
		{
		if(delay1000ms>500)OLED_ShowString(1,14,"CH1");
		else OLED_ShowString(1,14,"   ");
		}
		if(set_ccr2!=0)
		{
		if(delay1000ms>500)OLED_ShowString(2,14,"CH2");
		else OLED_ShowString(2,14,"   ");
		}
		if(set_ccr3!=0)
		{
		if(delay1000ms>500)OLED_ShowString(3,14,"CH3");
		else OLED_ShowString(3,14,"   ");
		}
		if(set_ccr4!=0)
		{
		if(delay1000ms>500)OLED_ShowString(4,14,"CH4");
		else OLED_ShowString(4,14,"   ");
		}
		
		break;
		/*display mode为1时为pwm设置*/
		
		case 1:
			if(pwm_display==0)
			{
			OLED_ShowString(1,2,"Channel-1");
			OLED_ShowString(2,2,"Channel-2");			
			OLED_ShowString(3,2,"Channel-3");			
			OLED_ShowString(4,2,"Channel-4");			
		  OLED_ShowString(choose_flag,1,">");			
			}
			//pwm_display不等于0的情况
			else 
			{
			OLED_ShowString(1,1,"Channel-");
			OLED_ShowNum(1,9,pwmch,1);
			OLED_ShowString(2,1,"Freq=");
		  OLED_ShowString(3,1,"Duty=");
			OLED_ShowString(3,8,"%");
			OLED_ShowString(4,1,"CCR_Val=");
			OLED_ShowNum(2,6,freq,6);
			OLED_ShowString(2,12,"Hz");
				switch(pwmch)     //PWM CH对应tim2对应通道
				{
					case 1:OLED_ShowNum(3,6,set_ccr1,2);//占空比 
								 OLED_ShowNum(4,9,set_ccr1,3);
								 break;
					
					case 2:OLED_ShowNum(3,6,set_ccr2,2);
					       OLED_ShowNum(4,9,set_ccr2,3);
								 break;
					
					case 3:OLED_ShowNum(3,6,set_ccr2,2);
					       OLED_ShowNum(4,9,set_ccr2,3);
								 break;
					
					case 4:OLED_ShowNum(3,6,set_ccr4,2);
				         OLED_ShowNum(4,9,set_ccr4,3);
								 break;
				}
			
			}
		break;
			
			//模式2为时间显示
		case 2:
		OLED_ShowString(2,7,"-");
		OLED_ShowString(2,10,"-");
		OLED_ShowNum(2,5,hour,2);
		OLED_ShowNum(2,8,min,2);
		OLED_ShowNum(2,11,sec,2);
		OLED_ShowNum(3,1,time1s,2);
		
		break;
			
		//模式3为设置  设置一共6个选项，统一管理
		case 3:
        setting_display();
   			break;
		case 4:
			//其他模式
				Other_mode();
				break;
	}
		
}
/*备忘录
调节预分频器TIM_PrescalerConfig();
调节ccr值TIM_SetCompare2();
 占空比Duty = CCR / (ARR + 1)
ARR + 1 = 100 对应duty = ccr;
*/
void setting_display()
{	
				if(choose_flag<5)    //选择项目小于5
 				{OLED_ShowString(choose_flag,1,">");
				OLED_ShowString(1,2,"PSC_Config");
				OLED_ShowString(2,2,"CCR1_Config");
				OLED_ShowString(3,2,"CCR2_Config");
				OLED_ShowString(4,2,"CCR3_Config");
					if(OK_flag==0)
	      {OLED_ShowNum(2,15,ccr1,2);
				OLED_ShowNum(3,15,ccr2,2);
				OLED_ShowNum(4,15,ccr3,2);
				OLED_ShowNum(1,13,psc,4);}
					if(OK_flag==1)     //选中项目
				{
					switch(choose_flag-1)   //减1确保chooseflag与ccr号正好对应
					{
						case 0:
							if(flash_time>300)OLED_ShowNum(1,13,psc,4);
							else OLED_ShowString(1,13,"    ");break;
						case 1:
							if(flash_time>300)OLED_ShowNum(2,15,ccr1,2);
							else OLED_ShowString(2,15,"  ");break;
						case 2:
							if(flash_time>300)OLED_ShowNum(3,15,ccr2,2);
							else OLED_ShowString(3,15,"  ");break;
						case 3:
							if(flash_time>300)OLED_ShowNum(4,15,ccr3,2);
							else OLED_ShowString(4,15,"  ");break;
					}
				}
				}
				
				//当选择大于5时的显示逻辑
			if(choose_flag==5)
			{OLED_ShowString(4,1,">");
       OLED_ShowString(1,2,"CCR1_Config");
			 OLED_ShowString(2,2,"CCR2_Config");
			 OLED_ShowString(3,2,"CCR3_Config");
		   OLED_ShowString(4,2,"CCR4_Config");
				//以下为数值显示逻辑
				if(OK_flag==0)
				{OLED_ShowNum(1,15,ccr1,2);
				 OLED_ShowNum(2,15,ccr2,2);
				 OLED_ShowNum(3,15,ccr3,2);
				 OLED_ShowNum(4,15,ccr4,2);
				}
				if(OK_flag==1)    //只存在一种情况，选中ccr4的情况
				{
				if(flash_time>300)OLED_ShowNum(4,15,ccr4,2);
				else OLED_ShowString(4,15,"  ");
				}
			}
			//当显示选择大于6时的选择逻辑
			if(choose_flag==6)    //在chooseflag6选中 关于我
			{
				if(OK_flag==0)      //普通界面
				{OLED_ShowString(4,1,">");
				 OLED_ShowString(4,2,"About Me");
				 OLED_ShowString(1,2,"CCR2_Config");
				 OLED_ShowString(2,2,"CCR3_Config");
				 OLED_ShowString(3,2,"CCR4_Config");
         OLED_ShowNum(1,15,ccr2,2);
				 OLED_ShowNum(2,15,ccr3,2);
				 OLED_ShowNum(3,15,ccr4,2);}
				
				 if(OK_flag==1)   //关于界面
				{
					OLED_ShowString(1,1,"    ABOUT ME    ");
					OLED_ShowString(4,1,"Designer:Kz152cx");
					OLED_ShowString(3,1,"CPU:stm32f10x");
					OLED_ShowString(2,1,"Name:4CH_PWM_OUT");
				}
				
			}
}
//OLED_Clear();
void Other_mode()
{
	if(OK_flag==0)
	{     if(choose_flag<5)
				{
				OLED_ShowString(1,2,"Sin_mode");
				OLED_ShowString(2,2,"Cos_mode");
				OLED_ShowString(3,2,"Tan_mode");
				OLED_ShowString(4,2,"In_mode");
				OLED_ShowString(choose_flag,1,">");
				}
				if(choose_flag==5)
				{
				OLED_ShowString(1,2,"Cos_mode");
				OLED_ShowString(2,2,"Tan_mode");
				OLED_ShowString(3,2,"In_mode");
				OLED_ShowString(4,1,">");	
				OLED_ShowString(4,2,"Triangle");
				}
				if(choose_flag==6)
				{
				OLED_ShowString(1,2,"Tan_mode");
				OLED_ShowString(2,2,"In_mode");
				OLED_ShowString(4,1,">");	
				OLED_ShowString(3,2,"Triangle");  //正三角波形
				OLED_ShowString(4,2,"Sawtooth");  //锯齿状波形
				}
	}
	if(OK_flag==1)   //针对单一项目选中
	{			
		//统一显示，针对6种模式均这样显示
		    OLED_ShowString(2,6,"CH1");
				OLED_ShowString(2,14,"CH2");
				OLED_ShowString(3,6,"CH3");
				OLED_ShowString(3,14,"CH4");
    //这里不显示通道对应模式，只显示通道开关
				if(ch1!=0)OLED_ShowString(2,3,"ON");
				if(ch1==0)OLED_ShowString(2,3,"OFF");
			  
				if(ch2!=0)OLED_ShowString(2,11,"ON");
				if(ch2==0)OLED_ShowString(2,11,"OFF");
				
				if(ch3!=0)OLED_ShowString(3,3,"ON");
				if(ch3==0)OLED_ShowString(3,3,"OFF");
				
				if(ch4!=0)OLED_ShowString(3,11,"ON");
				if(ch4==0)OLED_ShowString(3,11,"OFF");
		
		switch(other_flag)    //统一显示选择条目
				{
					case 1:OLED_ShowString(2,2,">");break;//选中第一个
					case 2:OLED_ShowString(2,10,">");break;//选中第二个
					case 3:OLED_ShowString(3,2,">");break;//选中第三个
					case 4:OLED_ShowString(3,10,">");break;//选中第四个
				}
		switch(choose_flag)   
			//针对选择模式 正弦 余弦 正切 对数 正三角波 锯齿状波
		{
			case 1:OLED_ShowString(1,1,"    Sin_Wave    ");break;
			case 2:OLED_ShowString(1,1,"    Cos_Wave    ");break;
			case 3:OLED_ShowString(1,1,"    Tan_Wave    ");break;
			case 4:OLED_ShowString(1,1,"    InE_Wave    ");break;
			case 5:OLED_ShowString(1,1,"    Tri_Wave    ");break;
			case 6:OLED_ShowString(1,1,"    Saw_Wave    ");break;
		}
		
	}
}
//double sin (double x);
//double cos (double x);
//double tan (double x);
void other_mode_calc()  //设定一个值delay100ms在0-100之间往复循环
{
	//lne1 tri1 saw1分别要求是最终可用值
  //这里的思路是将返回值直接赋值给aar寄存器，达到更改占空比的效果
	sin1 = 51+(char)(50 * sin(delay100ms / 15.92));
	cos1 = 51+(char)(50 * cos(delay100ms / 15.92));
	tank = 51+(char)(tan(delay100ms/31.84));
	if(tank>100) tan1 = 0;
	else {tan1 = tank;}
	//三角波为两个函数类
	if(delay100ms<=50) tri1 = 2*delay100ms;//在时间0-50期间，完成0-100线性波
	if(delay100ms> 50) tri1 = 200 - 2 * delay100ms; 
	//锯齿波 saw1 一个时间节点放两个
	if(delay100ms<50) saw1 = 2*delay100ms;
	if(delay100ms==50) saw1 = 0; //骤降波形
	if(delay100ms> 50) saw1 = 2*(delay100ms-50);
	//自然对数函数波形
	if(delay100ms<=1) lne1 = 0; //小于1不合法
	if(delay100ms> 1) lne1 = (char)(10 * log(delay100ms));
	//对正弦函数值进行处理
	if(sinsetup[0]==1)TIM_SetCompare1(TIM2,sin1);
	if(sinsetup[1]==1)TIM_SetCompare2(TIM2,sin1);
	if(sinsetup[2]==1)TIM_SetCompare3(TIM2,sin1);
	if(sinsetup[3]==1)TIM_SetCompare4(TIM2,sin1);
	//对余弦函数值进行处理
	if(cossetup[0]==1)TIM_SetCompare1(TIM2,cos1);
	if(cossetup[1]==1)TIM_SetCompare2(TIM2,cos1);
	if(cossetup[2]==1)TIM_SetCompare3(TIM2,cos1);
	if(cossetup[3]==1)TIM_SetCompare4(TIM2,cos1);
	//对正切函数值进行处理
	if(tansetup[0]==1)TIM_SetCompare1(TIM2,tan1);
	if(tansetup[1]==1)TIM_SetCompare2(TIM2,tan1);
	if(tansetup[2]==1)TIM_SetCompare3(TIM2,tan1);
	if(tansetup[3]==1)TIM_SetCompare4(TIM2,tan1);
	//对自然对数函数值进行处理
	if(lnesetup[0]==1)TIM_SetCompare1(TIM2,lne1);
	if(lnesetup[1]==1)TIM_SetCompare2(TIM2,lne1);
	if(lnesetup[2]==1)TIM_SetCompare3(TIM2,lne1);
	if(lnesetup[3]==1)TIM_SetCompare4(TIM2,lne1);
	//对正三角波行处理
	if(trisetup[0]==1)TIM_SetCompare1(TIM2,tri1);
	if(trisetup[1]==1)TIM_SetCompare2(TIM2,tri1);
	if(trisetup[2]==1)TIM_SetCompare3(TIM2,tri1);
	if(trisetup[3]==1)TIM_SetCompare4(TIM2,tri1);
	//对锯齿波进行处理
	if(sawsetup[0]==1)TIM_SetCompare1(TIM2,saw1);
	if(sawsetup[1]==1)TIM_SetCompare2(TIM2,saw1);
	if(sawsetup[2]==1)TIM_SetCompare3(TIM2,saw1);
	if(sawsetup[3]==1)TIM_SetCompare4(TIM2,saw1);
}
void IWDG_Init()
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);	//独立看门狗写使能
	IWDG_SetPrescaler(IWDG_Prescaler_16);			//设置预分频为16
	IWDG_SetReload(2500 - 1);							//设置重装值为2499，独立看门狗的超时时间为1000ms
	IWDG_ReloadCounter();							//重装计数器，喂狗
	IWDG_Enable();									//独立看门狗使能
}


int main(void)
{
	//Timer3_Init();
	pwm_init();     //pwm输出初始化 定时器2
	OLED_Init();		//OLED初始化	
	keyinit();      //按键初始化
  start_display();    //启动初始化
	Timer_Init();		//定时中断初始化  定时器4
	OLED_Clear();  //OLED清屏
	IWDG_Init();   //看门狗初始化
	Encoder_Init();
	
	while (1)
	{
		;  //按键检测函数
		OLED_display();
		IWDG_ReloadCounter();						//重装计数器，喂狗
		other_mode_calc();    //对pwm等效电压实现波形转化
	}
}

void TIM4_IRQHandler(void)          //1ms进入一次
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)		//判断是否是TIM2的更新事件触发的中断
	{ Key_Slow_Down++;
		if(Key_Slow_Down==6) Key_Slow_Down = 0;
		Key_Proc();
		flash_time++;
		if(flash_time==600) flash_time=0;
		time1s++;
		if(time1s==1000){time1s=0;sec++;}
		delay100ms++;
		if(delay100ms==100)
		{delay100ms=0;}
		delay1000ms++;
		if(delay1000ms==1000) {delay1000ms = 0;OLED_Clear();}
		if(sec==60){sec=0;min++;}
		if(min==60){hour++;min=0;}
		
		scan_channel();
	
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);			
	}

}
void EXTI9_5_IRQHandler(void)        //外部中断
{
	if (EXTI_GetITStatus(EXTI_Line6) == SET)		
	{
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == 0)
		{
			if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == 0)	
			{
				Key1op();
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line6);		
	}
	
	
		if (EXTI_GetITStatus(EXTI_Line7) == SET)		
	{
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == 0)
		{
			if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == 0)		
			{
				Key2op();
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line7);		
	}
	
}

//终于写完啦QAQ~
//第一次写 欢迎锐评或指正