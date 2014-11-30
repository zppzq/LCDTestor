/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: PowerManager.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2011��04��05��
**����޸�����: 2011��04��05��
**��        ��: ��Դ����Դ�ļ�
*****************************************************************************************************************/
#include "includes.h"
#include "stdarg.h"
#include "bsp\PowerKey.h"
#include "DataManage.h"
#include "DataAcq.h"
#include "CommApp.h"
#include "DS18B20\DS18B20.h"
#include "Bsp\LowPower.h"


/***********************************************************************************
�ɼ�����ʹ�����¹��緽ʽ��
1��3��5�ż��Ե�أ�
2��7V﮵�أ�������̫���ܵ�أ�
3��12V﮵�أ�
4��5V��9V��12V��ѹ��Դ��

��ʹ�õĵ�ѹ��ΧΪ��3.6 ~ 6; 7 ~ 10; 11.5 ~ 14
������ѹ������Ϊ�͵�ѹ��

***********************************************************************************/
//ʹ��3��5�ŵ��ʱ
#define	BATTERY_4V_MAX		6.0f		//��������ֵ
#define BATTERY_4V_FULL		4.6f		//100%������ѹֵ
#define	BATTERY_4V_LOW		3.6f		//0%������ѹֵ

//ʹ��7V﮵��ʱ
#define	BATTERY_7V_MAX		10.0f		//��������ֵ
#define BATTERY_7V_FULL		8.0f		//100%������ѹֵ
#define	BATTERY_7V_LOW		7.0f		//0%������ѹֵ

//ʹ��12V﮵��ʱ
#define	BATTERY_12V_MAX		14.0f		//��������ֵ
#define BATTERY_12V_FULL	12.5f		//100%������ѹֵ
#define	BATTERY_12V_LOW		11.5f		//0%������ѹֵ

//��ؼ���·��ѹϵ��
#define BATTERY_K			11.0f

//�Ƿ��ڵ͵�ѹ״̬
BOOL	m_bLowPower;

//��ص����ٷֱȣ���λ%
int8 	m_nBatteryRate;

//��ѹ������
float	m_fBatteryHigh;					//�������ѹ����λV
float	m_fBatteryLow;					//��ص͵�ѹ����λV

//��ѹ�ۼƴ���
uint32 	m_nVolLoCount;					//�͵�ѹ���� 
uint32 	m_nVolHiCount;					//�ߵ�ѹ����



//��ѹ�����ʼ��
void PowerManagerInit(void)
{
	//��ʼ��ѹ�ʼ���ϵ��
	m_fBatteryHigh = BATTERY_7V_FULL;
	m_fBatteryLow = BATTERY_4V_LOW;

	m_nBatteryRate = 100;
	m_bLowPower = FALSE;

	m_nVolLoCount = 0;
	m_nVolHiCount = 0;
}

//���õ�ѹ��Χ
void SetPowerRange(float fLo, float fHi)
{
	m_fBatteryLow = fLo;
	m_fBatteryHigh = fHi;
}

//��ȡ��ѹ�ٷֱ�����
int8 GetPowerRate()
{
	return m_nBatteryRate;
}

//�ж��Ƿ�͵�ѹ
BOOL IsLowPower()
{
	return m_bLowPower;
}

//�豸�������
void PowerTest() reentrant
{
	uint8 			i;
	float 			fVol = 0.0f;		
	uint32			nLedWinkIndex;
	float 			fBatteryVol;

	//����ص�ѹ
	i = 10;
	m_nVolLoCount = 0;
 	while(i--)
	{
		//��ȡADֵ
		fVol = ADPowerGetVol();
	   
		//����ʵ�ʵ�ѹ
		fVol *= BATTERY_K;
		
		//����������ѹ��
		fVol += 0.02f;
		fBatteryVol = fVol;


		//�Լ�ʱ�жϷ���
		if(fBatteryVol < BATTERY_4V_MAX)
		{
			m_fBatteryHigh = BATTERY_4V_FULL;
			m_fBatteryLow = BATTERY_4V_LOW;
		}
		else if(fBatteryVol < BATTERY_7V_MAX)
		{
			m_fBatteryHigh = BATTERY_7V_FULL;
			m_fBatteryLow = BATTERY_7V_LOW;
		}
 		else
		{
			m_fBatteryHigh = BATTERY_12V_FULL;
			m_fBatteryLow = BATTERY_12V_LOW;
		}

		//�ɼ�����ѹ�Ƿ������ͷ�ֵ============================
		if(fBatteryVol < m_fBatteryLow)
		{
			m_nVolLoCount++; 
		}
	}

	//��ѹ������ͷ�ֵ(BATTERY_4V_LOW)��һֱ���ߣ����ָ�========================
	if(m_nVolLoCount >= 5)
	{
		//�ض���ĵ�Դ
		#ifdef _SI4432_H_
		Si4432Close();
		#endif 

		#ifdef _MCU_ENTER_SLEEP_
		MCUEnterSleep();	
		#endif

		//��˸10����
		for(nLedWinkIndex = 0; nLedWinkIndex < 30; nLedWinkIndex++)
		{
			PowerLightOn();
			OSTimeDly(1);
			PowerLightOff();
			OSTimeDly(33);
		}

		//һֱ����
		while(1)
		{
			//�رմ洢��
			DiskClose();	

			#ifdef _MCU_ENTER_SLEEP_
			MCUEnterSleep();	
			#endif
			
			//����
			MCUSleep();
		}

	}
}


//��Դ��������
void PowerManagerProcess(void)
{
	float 			fVol = 0.0f;
	float 			fTmp;
 	uint32 			nLedWinkIndex; 	

	//��ȡADֵ
	fVol = ADPowerGetVol();
   
	//����ʵ�ʵ�ѹ
	fVol *= BATTERY_K;
	
	//����������ѹ��
	fVol += 0.02f;

	//ת��Ϊ�ٷֱ�
	fTmp = (fVol - m_fBatteryLow) / (m_fBatteryHigh - m_fBatteryLow) * 100;	
	if(fTmp < 0.0f) fTmp = 0.0f;	
	if(fTmp > 100.0f) fTmp = 100.0f;
	
	m_nBatteryRate = (int8)fTmp;
	if(m_nBatteryRate < 0) m_nBatteryRate = 0;
	else if(m_nBatteryRate > 100) m_nBatteryRate = 100;
	
	//�ж��Ƿ�͵�ѹ================================================================
	if(m_nBatteryRate < 5)
	{
		m_nVolLoCount++;
		if(m_nVolLoCount > 20)			
		{
			//���������͵�ѹ��������20�Σ��ж�Ϊ�͵�ѹ
			m_nVolLoCount = 20;
			m_bLowPower = TRUE;
		}
		else
		{
			//������ʼ��һ�μ��
			return;
		}
	}
	else
	{
		//�����ۼ�ֵ
		m_nVolLoCount = 0;
		m_bLowPower = FALSE;
	}


	//��Ӧ��״̬����==============================================================
	//�е�״̬
	if(m_bLowPower == FALSE)
	{
		//�ȴ�5�����ټ��
		OSTimeDly(OS_TICKS_PER_SEC*5);
	} 
	else
	{
		//�͵�ѹ����

		//Ԥ��20���ӣ��ȴ��������񶼴�����
		for(nLedWinkIndex = 0; nLedWinkIndex < 60; nLedWinkIndex++)
		{
			PowerLightOn();
			OSTimeDly(1);
			PowerLightOff();
			OSTimeDly(33);
		}

		//����(��Լ10����)
		for(nLedWinkIndex = 0; nLedWinkIndex < 1800; nLedWinkIndex++)
		{
			PowerLightOn();
			OSTimeDly(1);
			PowerLightOff();
			OSTimeDly(33);
		}

		//һֱ���ߣ�ֱ���ɼ����е�
		m_nVolHiCount = 0;
		while(1)
		{
			//��ȡADֵ
			fVol = ADPowerGetVol();
		   
			//����ʵ�ʵ�ѹ
			fVol *= BATTERY_K;
			
			//����������ѹ��
			fVol += 0.02f;

			//ת��Ϊ�ٷֱ�
			fTmp = (fVol - m_fBatteryLow) / (m_fBatteryHigh - m_fBatteryLow) * 100;		
			if(fTmp < 0.0f) fTmp = 0.0f;	
			if(fTmp > 100.0f) fTmp = 100.0f;

			m_nBatteryRate = (int8)fTmp;
			if(m_nBatteryRate < 0) m_nBatteryRate = 0;
			else if(m_nBatteryRate > 100) m_nBatteryRate = 100;
			
			//�ж��Ƿ�͵�ѹ
			if(m_nBatteryRate > 10)
			{
				m_nVolHiCount++;

				//ͳ���е�Ĵ���������Ҫ��
				if(m_nVolHiCount > 20)
				{
					m_nVolHiCount = 20;
					m_bLowPower = FALSE;

					//�ɼ����е�
					break;
				}
			}
			else
			{
				m_nVolHiCount = 0;
				m_bLowPower = TRUE;
			}

			#ifdef _MCU_ENTER_SLEEP_
			MCUEnterSleep();	
			#endif
			
			//����
			MCUSleep();

			#ifdef _MCU_ENTER_SLEEP_
			MCUExitSleep();	
			#endif
		}  
	}
}

