/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: PowerManager.c
**创   建   人: 杨承凯
**创 建 日  期: 2011年04月05日
**最后修改日期: 2011年04月05日
**描        述: 电源管理源文件
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
采集器可使用如下供电方式：
1、3节5号碱性电池；
2、7V锂电池；（包括太阳能电池）
3、12V锂电池；
4、5V、9V、12V稳压电源。

可使用的电压范围为：3.6 ~ 6; 7 ~ 10; 11.5 ~ 14
其他电压均设置为低电压。

***********************************************************************************/
//使用3节5号电池时
#define	BATTERY_4V_MAX		6.0f		//分区上限值
#define BATTERY_4V_FULL		4.6f		//100%电量电压值
#define	BATTERY_4V_LOW		3.6f		//0%电量电压值

//使用7V锂电池时
#define	BATTERY_7V_MAX		10.0f		//分区上限值
#define BATTERY_7V_FULL		8.0f		//100%电量电压值
#define	BATTERY_7V_LOW		7.0f		//0%电量电压值

//使用12V锂电池时
#define	BATTERY_12V_MAX		14.0f		//分区上限值
#define BATTERY_12V_FULL	12.5f		//100%电量电压值
#define	BATTERY_12V_LOW		11.5f		//0%电量电压值

//电池检测电路分压系数
#define BATTERY_K			11.0f

//是否处于低电压状态
BOOL	m_bLowPower;

//电池电量百分比，单位%
int8 	m_nBatteryRate;

//电压监测参数
float	m_fBatteryHigh;					//电池满电压，单位V
float	m_fBatteryLow;					//电池低电压，单位V

//电压累计次数
uint32 	m_nVolLoCount;					//低电压次数 
uint32 	m_nVolHiCount;					//高电压次数



//电压管理初始化
void PowerManagerInit(void)
{
	//初始电压率计算系数
	m_fBatteryHigh = BATTERY_7V_FULL;
	m_fBatteryLow = BATTERY_4V_LOW;

	m_nBatteryRate = 100;
	m_bLowPower = FALSE;

	m_nVolLoCount = 0;
	m_nVolHiCount = 0;
}

//设置电压范围
void SetPowerRange(float fLo, float fHi)
{
	m_fBatteryLow = fLo;
	m_fBatteryHigh = fHi;
}

//获取电压百分比余量
int8 GetPowerRate()
{
	return m_nBatteryRate;
}

//判断是否低电压
BOOL IsLowPower()
{
	return m_bLowPower;
}

//设备开启检测
void PowerTest() reentrant
{
	uint8 			i;
	float 			fVol = 0.0f;		
	uint32			nLedWinkIndex;
	float 			fBatteryVol;

	//检测电池电压
	i = 10;
	m_nVolLoCount = 0;
 	while(i--)
	{
		//获取AD值
		fVol = ADPowerGetVol();
	   
		//计算实际电压
		fVol *= BATTERY_K;
		
		//加上三极管压降
		fVol += 0.02f;
		fBatteryVol = fVol;


		//自检时判断分区
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

		//采集器电压是否低于最低阀值============================
		if(fBatteryVol < m_fBatteryLow)
		{
			m_nVolLoCount++; 
		}
	}

	//电压低于最低阀值(BATTERY_4V_LOW)，一直休眠，不恢复========================
	if(m_nVolLoCount >= 5)
	{
		//关额外的电源
		#ifdef _SI4432_H_
		Si4432Close();
		#endif 

		#ifdef _MCU_ENTER_SLEEP_
		MCUEnterSleep();	
		#endif

		//闪烁10秒钟
		for(nLedWinkIndex = 0; nLedWinkIndex < 30; nLedWinkIndex++)
		{
			PowerLightOn();
			OSTimeDly(1);
			PowerLightOff();
			OSTimeDly(33);
		}

		//一直休眠
		while(1)
		{
			//关闭存储器
			DiskClose();	

			#ifdef _MCU_ENTER_SLEEP_
			MCUEnterSleep();	
			#endif
			
			//休眠
			MCUSleep();
		}

	}
}


//电源管理事务
void PowerManagerProcess(void)
{
	float 			fVol = 0.0f;
	float 			fTmp;
 	uint32 			nLedWinkIndex; 	

	//获取AD值
	fVol = ADPowerGetVol();
   
	//计算实际电压
	fVol *= BATTERY_K;
	
	//加上三极管压降
	fVol += 0.02f;

	//转换为百分比
	fTmp = (fVol - m_fBatteryLow) / (m_fBatteryHigh - m_fBatteryLow) * 100;	
	if(fTmp < 0.0f) fTmp = 0.0f;	
	if(fTmp > 100.0f) fTmp = 100.0f;
	
	m_nBatteryRate = (int8)fTmp;
	if(m_nBatteryRate < 0) m_nBatteryRate = 0;
	else if(m_nBatteryRate > 100) m_nBatteryRate = 100;
	
	//判断是否低电压================================================================
	if(m_nBatteryRate < 5)
	{
		m_nVolLoCount++;
		if(m_nVolLoCount > 20)			
		{
			//连续测量低电压次数大于20次，判断为低电压
			m_nVolLoCount = 20;
			m_bLowPower = TRUE;
		}
		else
		{
			//立即开始下一次检测
			return;
		}
	}
	else
	{
		//清零累计值
		m_nVolLoCount = 0;
		m_bLowPower = FALSE;
	}


	//相应的状态处理==============================================================
	//有电状态
	if(m_bLowPower == FALSE)
	{
		//等待5秒钟再监测
		OSTimeDly(OS_TICKS_PER_SEC*5);
	} 
	else
	{
		//低电压处理

		//预闪20秒钟，等待其他任务都处理完
		for(nLedWinkIndex = 0; nLedWinkIndex < 60; nLedWinkIndex++)
		{
			PowerLightOn();
			OSTimeDly(1);
			PowerLightOff();
			OSTimeDly(33);
		}

		//闪灯(大约10分钟)
		for(nLedWinkIndex = 0; nLedWinkIndex < 1800; nLedWinkIndex++)
		{
			PowerLightOn();
			OSTimeDly(1);
			PowerLightOff();
			OSTimeDly(33);
		}

		//一直休眠，直到采集器有电
		m_nVolHiCount = 0;
		while(1)
		{
			//获取AD值
			fVol = ADPowerGetVol();
		   
			//计算实际电压
			fVol *= BATTERY_K;
			
			//加上三极管压降
			fVol += 0.02f;

			//转换为百分比
			fTmp = (fVol - m_fBatteryLow) / (m_fBatteryHigh - m_fBatteryLow) * 100;		
			if(fTmp < 0.0f) fTmp = 0.0f;	
			if(fTmp > 100.0f) fTmp = 100.0f;

			m_nBatteryRate = (int8)fTmp;
			if(m_nBatteryRate < 0) m_nBatteryRate = 0;
			else if(m_nBatteryRate > 100) m_nBatteryRate = 100;
			
			//判断是否低电压
			if(m_nBatteryRate > 10)
			{
				m_nVolHiCount++;

				//统计有电的次数已满足要求
				if(m_nVolHiCount > 20)
				{
					m_nVolHiCount = 20;
					m_bLowPower = FALSE;

					//采集器有电
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
			
			//休眠
			MCUSleep();

			#ifdef _MCU_ENTER_SLEEP_
			MCUExitSleep();	
			#endif
		}  
	}
}

