/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: F340AD.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��11��2��
**����޸�����: 2007��11��2��
**��        ��: 
********************************************************************************************************/
#define	_F340_AD_C_

//ͷ�ļ�-------------------------------------------------------------
#include "includes.h"

//micro--------------------------------------------------------------
#define		MDCLK			((long)2000000)		//SARת��ʱ��
//**********************���ݶ���*********************************************
//static INT8U	xdata	ADC0CLK;
//static INT8U	xdata	k;			//������ʱ
#define ADC1_DR_Address    ((u32)0x4001244C)
__IO uint16_t ADCConvertedValue;
					  
//��Դ�������ؿ���
#define	PW_AD_EN_PORT	GPIOC
#define	PW_AD_EN		0

//��������˿�
#define	PW_AD_PORT		GPIOC
#define	PW_AD			1
/*
#if	(PCB_VERSION == SJ_RFQC_SC_4D_3_3)		//3.3��

#define	PW_AD_EN_PORT	PORT(2)
#define	PW_AD_EN		BIT(6)

//��������˿�
#define	PW_AD_PORT		PORT(2)
#define	PW_AD			BIT(5)
#endif*/

#define ADC_PIN		GPIO_Pin_1
#define ADC_PORT	GPIOC

/******************************************************************************
**��	�ƣ�AD_Init()																 
**��	�ܣ�AD��ʼ��																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
void ADC_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
//	ErrorStatus 		 HSEStartUpStatus;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	/* Configure PC.04 (ADC Channel14) as analog input -------------------------*/
	GPIO_InitStructure.GPIO_Pin = ADC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC_PORT, &GPIO_InitStructure);
	MakePushPull(PW_AD_EN);
	SetLo(PW_AD_EN);

	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channel11 configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_55Cycles5);

// 	/* Enable ADC1 DMA */
// 	ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
	 
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
}
#ifdef AD_INIT
void ADInit(void) reentrant
{
	ADC_Configuration();
}
#endif

/******************************************************************************
**��	�ƣ�GetADValue()																 
**��	�ܣ�AD��ʼ��																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_AD_VALUE
unsigned int GetADValue(void) reentrant
{
	//����ѹ����Դ
	MakePushPull(PW_AD_EN);
	SetHi(PW_AD_EN);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	OSTimeDly(1);
	ADCConvertedValue = ADC_GetConversionValue(ADC1);
	SetLo(PW_AD_EN);
	MakeOpenDrain(PW_AD_EN);
	return ADCConvertedValue;
}
#endif

/******************************************************************************
**��	�ƣ�GetADVolValue()																 
**��	�ܣ�AD��ʼ��																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_AD_VOL_VALUE
float GetADVolValue(void) reentrant
{
	float f;
	unsigned int n;
	n = GetADValue();

	f = (float)n * 3.3f / 4096.0f;

	return f;
}
#endif

/******************************************************************************
**��	�ƣ�GetPowerVolValue()																 
**��	�ܣ�																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_POWER_VOL_VALUE
float GetPowerVolValue(void) reentrant
{
	float f;
	unsigned int n = GetADValue();

	f = (float)n * 2.49f / 4096.0f;

	f *= 11;

	return f;
}
#endif


/******************************************************************************
**��	�ƣ�GetTemperValue()																 
**��	�ܣ�																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_TEMPER_VALUE
float GetTemperValue(void) reentrant
{
	float f;
	unsigned int n = GetADValue();

	f = (float)n/4095*2.49;

	f *= 11;

	return f;
}
#endif


/******************************************************************************
**��	�ƣ�CalCurrentValue()																 
**��	�ܣ��������ֵ																 
**��ڲ�����AD�Ĳ���ֵ																 
**���ڲ���������ֵ															 
**˵	������														 
******************************************************************************/
#ifdef CAL_CURRENT_VALUE
float CalCurrentValue(unsigned int nADValue) reentrant
{
	float temp;
	float fCurrentValue = 0.0;
	temp = ((float)nADValue/4096)*VOL_REF;

	//�������ֵ
	fCurrentValue = ((float)nADValue/4096)*VOL_REF/3.7445/CalResValue(CurrentValue);
	return fCurrentValue*1.06;
}
#endif


void Stm32ADCDisable(void)
{
	ADC_Cmd(ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);	
}

void Stm32ADCEnable(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	
	ADC_Cmd(ADC1, ENABLE);
}

