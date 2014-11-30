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
#include "STM32ADC.h"

//micro--------------------------------------------------------------
#define		MDCLK			((long)2000000)		//SARת��ʱ��
//**********************���ݶ���*********************************************
static INT8U	xdata	ADC0CLK;
static INT8U	xdata	k;			//������ʱ
#define ADC1_DR_Address    ((u32)0x4001244C)
__IO uint16_t ADCConvertedValue;
					  

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
	DMA_InitTypeDef DMA_InitStructure;
	ErrorStatus 		 HSEStartUpStatus;

	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);

	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channel14 configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_55Cycles5);

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);

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

u32 GetADCValue(void)
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	return ADCConvertedValue;
}


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
	return GetADCValue();
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

	f = (float)n * 2.49f / 4096.0f;

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


