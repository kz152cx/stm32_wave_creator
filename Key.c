#include "stm32f10x.h"                  // Device header
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
			
