#ifndef __KEY_H
#define __KEY_H



#endif
void pwm_init(void)    //pa3��ʱ�����ã���������ch1 2 3ͨ�� ��Ӧpa012��
{
	/*����ʱ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//����TIM2��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//����GPIOA��ʱ��
	/*GPIO��ʼ��*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;     //����Ϊ�����������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							//��PA011���ų�ʼ��Ϊ�����������	
	/*����ʱ��Դ*/
	TIM_InternalClockConfig(TIM2);		//ѡ��TIM2Ϊ�ڲ�ʱ�ӣ��������ô˺�����TIMĬ��ҲΪ�ڲ�ʱ��
	/*ʱ����Ԫ��ʼ��*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//����ṹ�����
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //ʱ�ӷ�Ƶ��ѡ�񲻷�Ƶ���˲������������˲���ʱ�ӣ���Ӱ��ʱ����Ԫ����
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //������ģʽ��ѡ�����ϼ���
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;				//�������ڣ���ARR��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;				//Ԥ��Ƶ������PSC��ֵ
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //�ظ����������߼���ʱ���Ż��õ�
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);             //���ṹ���������TIM_TimeBaseInit������TIM2��ʱ����Ԫ
	/*����Ƚϳ�ʼ��*/ //oc Out Compare
	TIM_OCInitTypeDef TIM_OCInitStructure;							//����ṹ�����
	TIM_OCStructInit(&TIM_OCInitStructure);      //�ṹ���ʼ�������ṹ��û��������ֵ,����ṹ�岻�������µĴ���
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               //����Ƚ�ģʽ��ѡ��PWMģʽ1
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       //������ԣ�ѡ��Ϊ�ߣ���ѡ����Ϊ�ͣ�������ߵ͵�ƽȡ��
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   //���ʹ��
	TIM_OCInitStructure.TIM_Pulse = 0;								//��ʼ��CCRֵ
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);                        //���ṹ���������TIM_OC2Init������TIM2������Ƚ�ͨ��2
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);                        //���ĸ�ͨ������������
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);
	/*TIMʹ��*/
	TIM_Cmd(TIM2, ENABLE);			//ʹ��TIM2����ʱ����ʼ����
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
void keyinit()      //������ʼ������  ����pb0 pb1 pa3 pa5
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
			case 1:  //����1��ѡ��Ӽ�
			OLED_Clear();
			
			if(display_mode==3&&OK_flag==0)      //����ģʽ  //���ҽ���δ��������ģʽ����ʾģʽΪ3ʱ��Ч
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
	    case 2:      //����2��ѡ�����       //����ģʽ�£����ҽ���δ��������ģʽ����ʾģʽΪ3ʱ��Ч
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

			case 3:      //displaymode=0ʱΪok����
			OLED_Clear();
			

			
			if(display_mode==0)
				{
						switch(choose_flag)
						{ case 2:display_mode=1;choose_flag = 1;break;
							case 3:display_mode=2;choose_flag = 1;break;
							case 4:display_mode=3;//����setting�л�������
							//��ϵͳ�Ĵ����ж�ȡ����ͨ��ccr��ֵ
							set_ccr1 = TIM_GetCapture1(TIM2);
							set_ccr2 = TIM_GetCapture2(TIM2);
							set_ccr3 = TIM_GetCapture3(TIM2);
							set_ccr4 = TIM_GetCapture4(TIM2);
							set_psc = TIM_GetPrescaler(TIM2);
							//����ȡ��ֵ�����ڲ�����setccr,�漴��ֵ����ʾccr,
							ccr1 = set_ccr1;ccr2 = set_ccr2;
							ccr3 = set_ccr3;ccr4 = set_ccr4;
							psc = set_psc;
							choose_flag = 1;break;}
						break;
			  }			
			
				if(display_mode==1)               //pwmֱ����ʾ/����
			{ 
				
				pwmch = choose_flag;
				pwm_display = 1;                  //����pwm��ʾ ����ȡʵʱpwm��Ӧ������ֵ
				set_ccr1 = TIM_GetCapture1(TIM2);
				set_ccr2 = TIM_GetCapture2(TIM2);
				set_ccr3 = TIM_GetCapture3(TIM2);
				set_ccr4 = TIM_GetCapture4(TIM2);
				set_psc = TIM_GetPrescaler(TIM2);
				freq = 720000 / (set_psc + 1);
				break;
			}
			
			if(display_mode==3&&OK_flag==0)   //ȷ���߼�
			{OLED_Clear();OK_flag=1;flash_flag=1;break;}
			
			if(display_mode==3&&OK_flag==1)   //��ʱ����Ϊ�����߼�
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
			//����ȫΪ�˳��߼� ���漰��������
			if(display_mode==1&&pwm_display==1){pwm_display=0;break;}
			if(display_mode==1&&pwm_display==0){display_mode=0;choose_flag=2;break;}
			if(display_mode==3&&OK_flag==0){display_mode=0;choose_flag=2;break;}
			if(display_mode==3&&OK_flag==1){OK_flag=0;flash_flag=0;break;}
			if(display_mode==2){display_mode = 0;choose_flag=2;break;}
			
			
			break;
			}

      }  //if��
			


void OLED_display(void)
{
	//ģʽ0Ϊ�˵�ѡ��ģʽ
	switch(display_mode)
	{ 
		case 0:                                  
		OLED_ShowString(1,1,"    MainMenu");  //���˵�
		OLED_ShowString(2,2,"PWM Set");           //pwm����
		OLED_ShowString(3,2,"Clock Test");             //ʱ��
		OLED_ShowString(4,2,"Setting");           //����
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
		/*display modeΪ1ʱΪpwm����*/
		
		case 1:
			if(pwm_display==0)
			{
			OLED_ShowString(1,2,"Channel-1");
			OLED_ShowString(2,2,"Channel-2");			
			OLED_ShowString(3,2,"Channel-3");			
			OLED_ShowString(4,2,"Channel-4");			
		  OLED_ShowString(choose_flag,1,">");			
			}
			//pwm_display������0�����
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
				switch(pwmch)     //PWM CH��Ӧtim2��Ӧͨ��
				{
					case 1:OLED_ShowNum(3,6,set_ccr1,2);//ռ�ձ� 
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
			
			//ģʽ2Ϊʱ����ʾ
		case 2:
		OLED_ShowString(2,7,"-");
		OLED_ShowString(2,10,"-");
		OLED_ShowNum(2,5,hour,2);
		OLED_ShowNum(2,8,min,2);
		OLED_ShowNum(2,11,sec,2);
		OLED_ShowNum(3,1,time1s,2);
		
		break;
			
		//ģʽ3Ϊ����  ����һ��6��ѡ�ͳһ����
		case 3:
        setting_display();
   			break;
	}
		
}
/*����¼
����Ԥ��Ƶ��TIM_PrescalerConfig();
����ccrֵTIM_SetCompare2();
 ռ�ձ�Duty = CCR / (ARR + 1)
ARR + 1 = 100 ��Ӧduty = ccr;
*/
void setting_display()
{	
				if(choose_flag<5)    //ѡ����ĿС��5
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
					if(OK_flag==1)     //ѡ����Ŀ
				{
					switch(choose_flag-1)   //��1ȷ��chooseflag��ccr�����ö�Ӧ
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
				
				//��ѡ�����5ʱ����ʾ�߼�
			if(choose_flag==5)
			{OLED_ShowString(4,1,">");
       OLED_ShowString(1,2,"CCR1_Config");
			 OLED_ShowString(2,2,"CCR2_Config");
			 OLED_ShowString(3,2,"CCR3_Config");
		   OLED_ShowString(4,2,"CCR4_Config");
				//����Ϊ��ֵ��ʾ�߼�
				if(OK_flag==0)
				{OLED_ShowNum(1,15,ccr1,2);
				 OLED_ShowNum(2,15,ccr2,2);
				 OLED_ShowNum(3,15,ccr3,2);
				 OLED_ShowNum(4,15,ccr4,2);
				}
				if(OK_flag==1)    //ֻ����һ�������ѡ��ccr4�����
				{
				if(flash_time>300)OLED_ShowNum(4,15,ccr4,2);
				else OLED_ShowString(4,15,"  ");
				}
			}
			//����ʾѡ�����6ʱ��ѡ���߼�
			if(choose_flag==6)    //��chooseflag6ѡ�� ������
			{
				if(OK_flag==0)      //��ͨ����
				{OLED_ShowString(4,1,">");
				 OLED_ShowString(4,2,"About Me");
				 OLED_ShowString(1,2,"CCR2_Config");
				 OLED_ShowString(2,2,"CCR3_Config");
				 OLED_ShowString(3,2,"CCR4_Config");
         OLED_ShowNum(1,15,ccr2,2);
				 OLED_ShowNum(2,15,ccr3,2);
				 OLED_ShowNum(3,15,ccr4,2);}
				
				 if(OK_flag==1)   //���ڽ���
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
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);	//�������Ź�дʹ��
	IWDG_SetPrescaler(IWDG_Prescaler_16);			//����Ԥ��ƵΪ16
	IWDG_SetReload(2500 - 1);							//������װֵΪ2499���������Ź��ĳ�ʱʱ��Ϊ1000ms
	IWDG_ReloadCounter();							//��װ��������ι��
	IWDG_Enable();									//�������Ź�ʹ��
}


int main(void)
{
	
	pwm_init();     //pwm�����ʼ��
	OLED_Init();		//OLED��ʼ��	
	keyinit();      //������ʼ��
  start_display();    //������ʼ��
	Timer_Init();		//��ʱ�жϳ�ʼ��
	OLED_Clear();  //OLED����
	IWDG_Init();   //���Ź���ʼ��
	while (1)
	{
		Key_Proc();  //������⺯��
		OLED_display();
		IWDG_ReloadCounter();						//��װ��������ι��
	}
}

void TIM4_IRQHandler(void)          //1ms����һ��
{