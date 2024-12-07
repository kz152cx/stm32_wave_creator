#ifndef __KEY_H
#define __KEY_H



#endif
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
			OLED_Clear();
			
			if(display_mode==3&&OK_flag==0)      //设置模式  //当且仅当未处于设置模式且显示模式为3时有效
			{choose_flag++;
			if(choose_flag==7) choose_flag=6;}

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
			if(choose_flag==5)choose_flag=4;}
			
			if(display_mode==1)
			{choose_flag++;
			if(choose_flag==5)choose_flag=4;}	
			
			break;
	    case 2:      //按键2，选项减减       //设置模式下，当且仅当未处于设置模式且显示模式为3时有效
			OLED_Clear();	
			if(display_mode==0)
			{choose_flag--;
			if(choose_flag==1)choose_flag=2;}
			
			if(display_mode==1)
			{choose_flag--;
			if(choose_flag==0)choose_flag=1;}
			
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
			break;    

			case 3:      //displaymode=0时为ok按键
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
							choose_flag = 1;break;}
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
		  case 4: 
				OLED_Clear();
			//以下全为退出逻辑 不涉及保存或更新
			if(display_mode==1&&pwm_display==1){pwm_display=0;break;}
			if(display_mode==1&&pwm_display==0){display_mode=0;choose_flag=2;break;}
			if(display_mode==3&&OK_flag==0){display_mode=0;choose_flag=2;break;}
			if(display_mode==3&&OK_flag==1){OK_flag=0;flash_flag=0;break;}
			if(display_mode==2){display_mode = 0;choose_flag=2;break;}
			
			
			break;
			}

      }  //if的
			


void OLED_display(void)
{
	//模式0为菜单选择模式
	switch(display_mode)
	{ 
		case 0:                                  
		OLED_ShowString(1,1,"    MainMenu");  //主菜单
		OLED_ShowString(2,2,"PWM Set");           //pwm配置
		OLED_ShowString(3,2,"Clock Test");             //时钟
		OLED_ShowString(4,2,"Setting");           //设置
		OLED_ShowString(choose_flag,1,">");
		
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
	
	pwm_init();     //pwm输出初始化
	OLED_Init();		//OLED初始化	
	keyinit();      //按键初始化
  start_display();    //启动初始化
	Timer_Init();		//定时中断初始化
	OLED_Clear();  //OLED清屏
	IWDG_Init();   //看门狗初始化
	while (1)
	{
		Key_Proc();  //按键检测函数
		OLED_display();
		IWDG_ReloadCounter();						//重装计数器，喂狗
	}
}

void TIM4_IRQHandler(void)          //1ms进入一次
{