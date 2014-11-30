/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: Lights.c
**��   ��   ��: 
**�� �� ��  ��: 2008��9��17��
**����޸�����: 2008��9��17��
**��        ��: ��ɢ�ĺ���
*****************************************************************************************************************/
#define 	_LIGHTS_C_

//includes--------------------------------------------------------------------------------------------------------
#include "includes.h"
#include "CpuPortAccess.h"
#include "lights.h"

//�������
#define USE_LED

//signal defines--------------------------------------------------------------------------------------------------

//****************************************************************************************************************

//�˿��趨 
#define	LED_PORT			GPIOB	   
#define	LED					5

#define	POWER_LED_PORT		GPIOB	   
#define	POWER_LED			5


//ָʾ�ƿ��ƣ�0��ʾ�͵�ƽ���Ƶ���
#define	LED_ON		0

/****************************************************************************
* ��	�ƣ�SigLightOn()
* ��	�ܣ��źŵƵ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SigLightOn() reentrant
{
#if (LED_ON == 0)
	SetLo(LED);
#endif
}

/****************************************************************************
* ��	�ƣ�SigLightOff()
* ��	�ܣ��źŵ�Ϩ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SigLightOff() reentrant
{
#if (LED_ON == 0)	
	SetHi(LED);
#endif
}

void SigLightToggle() reentrant
{
	static u8 flag = 0;

	if (flag)
	{
		flag = 0;
		SigLightOn();
	}
	else
	{
		flag = 1;
		SigLightOff();
	}
}

/****************************************************************************
* ��	�ƣ�PowerLightOn()
* ��	�ܣ���Դ�Ƶ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void PowerLightOn(void) reentrant
{
	SetLo(POWER_LED);
}

/****************************************************************************
* ��	�ƣ�PowerLightOff()
* ��	�ܣ���Դ��Ϩ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void PowerLightOff(void) reentrant
{
	SetHi(POWER_LED);
}

/****************************************************************************
* ��	�ƣ�LightsInit()
* ��	�ܣ�ָʾ�Ƴ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void LightsInit() reentrant
{
	//ָʾ��
	SetHi(LED);
	SetHi(POWER_LED);
	MakeOpenDrain(LED);
	MakeOpenDrain(POWER_LED);
	
	//MakePushPull(LED);
	//�����ź���
}




/****************************************************************************
* ��	�ƣ�LightsProcess()
* ��	�ܣ�ָʾ�ƴ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void LightsProcess() reentrant
{

	{
		SigLightOn();
		SigLightOff(); 
	}
	{
//		if(IsLowPower() != TRUE)
		{
			PowerLightOn();
			PowerLightOff(); 
		}
	}
}













