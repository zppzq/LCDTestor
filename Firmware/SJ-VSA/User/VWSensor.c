/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------文 件 信 息--------------------------------------------------------**
**文   件   名： VWSensor.c												 	 **
**创   建   人： 蒋小明														 **
**创 建  时 间： 2008-09-28													 **
**最后修改时间：													 		 **
**描        述： 														 	 **
******************************************************************************/

#define	_VWSENSOR_C_

//--------Includes-------------------------------------------------------------
#include "..\sdk\bsp\bsp.h"
#include "VWSensor.h"

//#define VWS_DEBUG		1

//频率参数
static uint16 nFreqValidCount;					//采样频率中认为有效的频率个数
static int32 nLocalTemp;						//本地临时变量
static fp32 nFreqRecord = 400.0f; 				//记录上一次的谐振值
static fp32 fFreqAver;							//均值
static fp32 fFreqAverValid; 					//有效的均值
static fp32 fFreqWaken;							//激振频率
static fp32 fFreqWakenStep;						//激振增量
static uint16 nWakenIndex;						//激振的方波个数索引
static uint16 nWakenMax;						//激振最大次数
static float fFreqValidCheck;					//有效检测范围
static uint16 nSyntTimes;

//信号量
static OS_EVENT 	*pVWSEvent;					//事件控制块
static uint8 		nVWSErr;					//错误标志

static OS_EVENT 	*pTimer0Event;				//事件控制块
static uint8 		nTimer0Err;					//错误标志


static fp32			VWParamG = 3.7f;
static fp32			VWParamC = 1.027459f;

static fp32			VWf0;
static fp32			VWfx;
static int16		VWPress;

//电平有效检测引脚
#define OUT_EN_PORT		PORT(0)
#define OUT_EN			BIT(4)

//充电放电引脚
#define FLOW_OPEN_PORT	PORT(0)
#define FLOW_OPEN		BIT(5)

//激振控制引脚
#define VWS_WAKEN_PORT	PORT(0)
#define VWS_WAKEN		BIT(3)

/******************************************************************************
**名	称： VWSensorInit()
**功	能： 拾振初始化
**入口参数： 无
**出口参数： 无
**说	明： 无
******************************************************************************/
#ifdef _VW_SENSOR_INIT_
void Timer0Init() reentrant
{
	//时钟选择
	CKCON &= 0xF8;		
	
	//模式选择(16位定时器)
	TMOD &= 0xF0;
	TMOD |= 0x01;


	ET0 = 0;
	TR0 = 0;

	pTimer0Event = OSSemCreate(0);
}



void VWSensorInit() reentrant
{
	//电平有效检测
	MakeOpenDrain(OUT_EN);				// 设置输入

	//充放电引脚设置
	MakePushPull(FLOW_OPEN);
	SetLo(FLOW_OPEN);

	//激振引脚设置
	MakePushPull(VWS_WAKEN);
	SetLo(VWS_WAKEN);

	//信号量初始化
	pVWSEvent = OSSemCreate(0);			//创建信号量
	Timer0Init();						//定时器0初始化	
}
#endif

//设置参数
void VWSetParam(fp32* pParam) reentrant
{
	VWParamG = pParam[0];
	VWParamC = pParam[1];
}

//初始化配置
//nMilisecond: 0-600
void SetTimer0(uint16 nMilisecond) reentrant
{
	uint32 nTimeSet;
	uint16 nTimeSet16;

	nMilisecond = (nMilisecond > 600) ? 600 : nMilisecond;

	nTimeSet = SYSCLK / 1000;
 	nTimeSet *= nMilisecond;
	nTimeSet /= 480;

	nTimeSet16 = nTimeSet;
	nTimeSet16 = -nTimeSet16;

	TH0 = HIBYTE(nTimeSet16);
	TL0 = LOBYTE(nTimeSet16);

	//使能中断，运行
	ET0 = 1;
	TR0 = 1;

	//等待计时完毕
	OSSemPend(pTimer0Event, 0, &nTimer0Err);
}

void Timer0ISR() interrupt 1
{
	//进中断
	OSIntEnter();

	//禁止中断，运行
	ET0 = 0;
	TR0 = 0;
	
	//发送信号量
	OSSemPost(pTimer0Event);

	//出中断
	OSIntExit();

}

/******************************************************************************
**名	称： VWSWakenSetFreq()
**功	能： 设置频率
**入口参数： 需要设置的频率
**出口参数： 无
**说	明： 最大频率：nPCA0CLK/2，最小时钟：nPCA0CLK/2/65536
******************************************************************************/
void VWSWakenSetFreq(float fFreq) reentrant
{
	//工作范围
    fFreq = (fFreq < 400.0f) ? 400.0f : fFreq;
	fFreq = (fFreq > 6000.0f) ? 6000.0f : fFreq;

	//计算控制字
	nFreq16Span = (unsigned int)(nPCA0CLK / fFreq / 2);
}

/******************************************************************************
**名	称： VWSWakenStart()
**功	能： 开始激振
**入口参数： 无
**出口参数： 无
**说	明： 无
******************************************************************************/
void VWSWakenStart() reentrant
{
	if(nTestStatus == TEST_FREQ) nWakenMax = 500; 
	else nWakenMax = 300;
	nWakenIndex = 0;
	XBR1         |= 0x42;					// 使能CEX0、CEX1到端口引脚	
	nIntType      = INT_TYPE_WAKEN;			// 指明中断类型
	nSensorStatus = SENSOR_WAKEN;			// 进入激振状态
	EnableCCF1Int();						// PCA0CPM1 |= 0x01;						// 使能CEX1中断
	EnablePCA0();							// PCA0CN 	|= 0x40;						// 使能PCA0
}

/******************************************************************************
**名	称： VWSWakenWait()
**功	能： 等待激振结束
**入口参数： 无
**出口参数： 无
**说	明： 无
******************************************************************************/
void VWSWakenWait() reentrant
{
	OSSemPend(pVWSEvent, 0, &nVWSErr);
}


/******************************************************************************
**名	称： VWSCollectWait()
**功	能： 等待拾振结束
**入口参数： 无
**出口参数： 无
**说	明： 无
******************************************************************************/
BOOL VWSCollectWait() reentrant
{
	OSSemPend(pVWSEvent, 6, &nVWSErr);

	if(nVWSErr == OS_NO_ERR) 
	{
		if(nCollectOK == 1) return TRUE;
	}

	return FALSE;
}



/******************************************************************************
**名	称： VWSWakenStop()
**功	能： 停止拾振
**入口参数： 无
**出口参数： 无
**说	明： 无
******************************************************************************/
void VWSWakenStop() reentrant
{
	DisableCCF1Int();						// PCA0CPM1 &= ~0x01;						// 禁止CEX1中断
	XBR1 = 0x41;							// 禁止CEX1
	SetLo(VWS_WAKEN);						// 给传感器0电流
	DisablePCA0();							// PCA0CN &= ~0x40;	// 禁止PCA0
	nSensorStatus = SENSOR_IDLE;			// 进入空闲状态
}


/******************************************************************************
**名	称： Int1ISR()
**功	能： 拾振电平检测中断
**入口参数： 无
**出口参数： 无
**说	明： 进入此中断表示可以进行拾振，电平达到了一定强度
******************************************************************************/
void Int1ISR() interrupt 2
{
	nIntType      = INT_TYPE_COLLECT;	// 指明中断类型	
	nSensorStatus = SENSOR_COLLECT;		// 进入拾振状态
	nCollectIntCount  = 0;				// 中断次数清零
	nCollectOverCount = 0;				// PCA0溢出次数清零
	EnableCCF0Int();					// PCA0CPM0 |= 0x01;	// 开CEX0中断
	EnableCFInt();						// 使能PCA0溢出中断
	EnablePCA0();						// 使能PCA0计数器
	EX1 = 0;							// 禁止外部中断1
}


/******************************************************************************
**名	称： VWSCollectStart()
**功	能： 开始拾振
**入口参数： 无
**出口参数： 无
**说	明： 无
******************************************************************************/
void VWSCollectStart() reentrant
{
 	// 如果连续测频率，需要加上此段
	nIntType      = INT_TYPE_COLLECT;	// 指明中断类型	
	nSensorStatus = SENSOR_COLLECT;		// 进入拾振状态
	nCollectIntCount  = 0;				// 中断次数清零
	nCollectOverCount = 0;				// PCA0溢出次数清零
	PCA0CN &= 0x40;						// 清除中断标志
	EnableCCF0Int();					// PCA0CPM0 |= 0x01;	// 开CEX0中断
	EnableCFInt();						// 关PCA0溢出中断
	EnablePCA0();						// 使能PCA0计数器
}

/******************************************************************************
**名	称： VWSCollectStop()
**功	能： 停止拾振
**入口参数： 无
**出口参数： 无
**说	明： 无
******************************************************************************/
void VWSCollectStop() reentrant
{
	DisablePCA0();							// 禁止PCA0计数器
	DisableCFInt();							// 禁止PCA0溢出中断
	DisableCCF0Int();						// 禁止CEX0中断
	EX1 = 0;								// 禁止外部中断1
	nSensorStatus = SENSOR_IDLE;			// 进入拾振状态
}

/******************************************************************************
**名	称： VWSGetFreq()
**功	能： 获取拾振的频率
**入口参数： 无
**出口参数： 频率
**说	明： 测频率真实情况
******************************************************************************/
static BOOL TryLastFreg() reentrant
{
	//临时变量
	uint16 i;

	//激振
	VWSWakenSetFreq(nFreqRecord);		// 设置激振频率
	VWSWakenStart();					// 开始激振
	VWSWakenWait();						// 等待激振完毕
	VWSWakenStop();						// 关闭激振		

	//放电
	SetHi(FLOW_OPEN);
	SetTimer0(50);

    //充电
	SetLo(FLOW_OPEN);
	SetTimer0(300);
	
	
	//拾振开始，等待，停止	
	VWSCollectStart();
	VWSCollectWait();
	VWSCollectStop();
	
	//如果拾振有效
	if(nCollectOK == 1) 
	{
		//计算频率值
		nFreqValidCount = 0;
		fFreqAver = 0;
		fFreqAverValid = 0;
		for(i=0; i<VWS_COLLECT_COUNT; i++)
		{
			fFreq[i]  = (float)nPCA0CLK;
			fFreq[i] /=	(float)((nCollectOverV[i+1] - nCollectOverV[i])* 65536 + nCollectValue[i+1] - nCollectValue[i]);
			fFreqAver += fFreq[i];
		}
		fFreqAver /= VWS_COLLECT_COUNT;

		// 对频率进行筛选
		for(i=0; i<VWS_COLLECT_COUNT; i++)
		{
			if((fFreq[i] > fFreqAver - FREQ_VALID_LIMIT) && (fFreq[i] < fFreqAver + FREQ_VALID_LIMIT))
			{
				fFreq[nFreqValidCount] = fFreq[i];				// 重新组织有效频率
				fFreqAverValid += fFreq[nFreqValidCount];		// 记录有效频率
				nFreqValidCount++;								// 有效频率个数加1
			}
		}
					
		// 求平均值
		if(nFreqValidCount > 0)
		{
			//如果有效频率个数大于40个，平均频率在激振频率正负0.1范围内算是正确，测试结束
			if(nFreqValidCount >= VWS_COLLECT_COUNT)
			{
				// 换总体方式计算平均频率
				nLocalTemp = nCollectOverV[VWS_COLLECT_COUNT] - nCollectOverV[0];
				nLocalTemp = nLocalTemp<<16;
				nLocalTemp += nCollectValue[VWS_COLLECT_COUNT];
				nLocalTemp -= nCollectValue[0];

				fFreqAver = (float)nPCA0CLK;							
				fFreqAver *= VWS_COLLECT_COUNT;
				fFreqAver /= nLocalTemp;

				//新的激振值
				nFreqRecord = fFreqAver;

				return TRUE;
			}
		}
	}

	//返回结果
	return FALSE;

}

/******************************************************************************
**名	称： VWSSetZero()
**功	能： 调零
**入口参数： 无
**出口参数： 无
**说	明： 
******************************************************************************/
void VWSSetZero() reentrant
{
	do
	{
		VWf0 = VWSGetFreq();
	}while(VWf0 < 200.0f);
	
	
	VWf0 *= VWf0;

	//不使用本地调零，采用默认参考点
	VWf0 = 360000.0f;
}

/******************************************************************************
**名	称： VWSGetValue()
**功	能： 获取应变值
**入口参数： 无
**出口参数： 频率
**说	明： 测频率真实情况
******************************************************************************/
int16 VWSGetValue() reentrant
{
	do
	{
		VWfx = VWSGetFreq();
	}while(VWfx < 200.0f);
	
	
	VWfx *= VWfx;
	VWfx = VWfx - VWf0;
	VWfx = VWfx / 1000.0;
	VWfx = VWfx * VWParamG * VWParamC;
	
	VWPress = (int16)VWfx;

	return VWPress;
}

/******************************************************************************
**名	称： VWSGetFreq()
**功	能： 获取拾振的频率
**入口参数： 无
**出口参数： 频率
**说	明： 测频率真实情况
******************************************************************************/
//#define VW_NO_SENSOR_DEBUG		1
float VWSGetFreq() reentrant
{
	uint16 i;
	BOOL bFreqFind = 0;				   	// 是否找到频率
	nSyntTimes = 0;						// 谐振次数

#ifdef VW_NO_SENSOR_DEBUG
	OSTimeDly(OS_TICKS_PER_SEC * 2);
	return 800.0; Error Set
#endif

	//尝试上一次测量值	
 	//if(TRUE)
	if(TryLastFreg() == FALSE)
	{
		nTestStatus = SCAN_FREQ;		// PCA0激振方式选择，关系到激振方波个数
	
		//放电
		SetHi(FLOW_OPEN);
		SetTimer0(200);
		//SetLo(FLOW_OPEN);
	
		//粗调
		fFreqWaken = 400.0f;		    // 激振初值
		fFreqWakenStep = 50.0;			// 激振步进
		while(fFreqWaken < 4000.0f)
		{
			//激振	
			VWSWakenSetFreq(fFreqWaken);		// 设置最开始的激振频率
			VWSWakenStart();					// 以此频率激振
			VWSWakenWait();						// 等待激振完毕
			VWSWakenStop();						// 关闭激振
			
			//放电
			SetHi(FLOW_OPEN);
			SetTimer0(200);

#ifdef VWS_DEBUG	
			if(GetSignal(OUT_EN) == TRUE)			// 是否放完电
			{
				_nop_(); 
			}
#endif
	
		    //充电
			SetLo(FLOW_OPEN);
			SetTimer0(300);
	
			//检测有效电平
			if(GetSignal(OUT_EN) == FALSE)			// 是否充好电
			{
				_nop_();
				break;
			}
			
			//继续下一次扫描
			fFreqWaken += fFreqWakenStep;
		}

	}
	else
	{
		fFreqWaken = nFreqRecord;
	}
	

	//细调
	nTestStatus = TEST_FREQ;			  	// 激振方式选择，关系到激振方波个数
	
	// -------- 再通过激振扫频 ------------------------------------------------
	while(fFreqWaken <= VWS_MAX_FREQ)
	{
		//激振
		VWSWakenSetFreq(fFreqWaken);		// 设置激振频率
		VWSWakenStart();
		VWSWakenWait();						// 等待激振完毕
		VWSWakenStop();						// 关闭激振		

		//放电
		SetHi(FLOW_OPEN);
		SetTimer0(50);

	    //充电
		SetLo(FLOW_OPEN);
		SetTimer0(300);
		
		
		//拾振开始，等待，停止	
		VWSCollectStart();
		VWSCollectWait();
		VWSCollectStop();
		
		//如果拾振有效
		if(nCollectOK == 1) 
		{
			//计算频率值
			nFreqValidCount = 0;
			fFreqAver = 0;
			fFreqAverValid = 0;
			for(i=0; i<VWS_COLLECT_COUNT; i++)
			{
				fFreq[i]  = (float)nPCA0CLK /(float)((nCollectOverV[i+1] - nCollectOverV[i])* 65536
							 + nCollectValue[i+1] - nCollectValue[i]);
				fFreqAver += fFreq[i];
			}
			fFreqAver /= VWS_COLLECT_COUNT;

			// 对频率进行筛选
			for(i=0; i<VWS_COLLECT_COUNT; i++)
			{
				if((fFreq[i] > fFreqAver - FREQ_VALID_LIMIT) && (fFreq[i] < fFreqAver + FREQ_VALID_LIMIT))
				{
					fFreq[nFreqValidCount] = fFreq[i];				// 重新组织有效频率
					fFreqAverValid += fFreq[nFreqValidCount];		// 记录有效频率
					nFreqValidCount++;								// 有效频率个数加1
				}
			}
						
			// 求平均值
			if(nFreqValidCount > 0)
			{
				fFreqAver = fFreqAverValid / nFreqValidCount;			// 有效频率的平均值
				fFreqAverValid = fFreqAver;		

				// 如果有效频率个数大于40个，平均频率在激振频率正负0.1范围内算是正确，测试结束
				if(nFreqValidCount >= VWS_COLLECT_COUNT)
				{
					// 换总体方式计算平均频率
					nLocalTemp = nCollectOverV[VWS_COLLECT_COUNT] - nCollectOverV[0];
					nLocalTemp = nLocalTemp<<16;
					nLocalTemp += nCollectValue[VWS_COLLECT_COUNT];
					nLocalTemp -= nCollectValue[0];

					fFreqAver = (float)nPCA0CLK;							
					fFreqAver *= VWS_COLLECT_COUNT;
					fFreqAver /= nLocalTemp;

					if(fFreqAver > 1000.0f) fFreqValidCheck = 0.08f;
					if(fFreqAver < 600.0f) fFreqValidCheck = 0.2f;
					else fFreqValidCheck = 0.15;
					
					if((fFreqAver < fFreqWaken + fFreqValidCheck) && (fFreqAver > fFreqWaken - fFreqValidCheck))
					{
						bFreqFind = 1;					// 标识找到频率
						nFreqRecord = fFreqAver;		// 记录频率
						SetHi(FLOW_OPEN);				// 放电
						break;
					}
					else								// 测得的频率超出规定范围
					{
						// 将激振频率换成测到的有效频率
						fFreqWaken = fFreqAver;
						
						//防止死锁
						nSyntTimes++;
						if(nSyntTimes >= VWS_SYNT_MAXTIMES)
						{
							bFreqFind = 1;					// 标识找到频率
							nFreqRecord = fFreqAver;		// 记录频率
							SetHi(FLOW_OPEN);				// 放电
							break;
						}
					}
				}
				else										// 有效频率次数不够
				{
					fFreqWaken += FREQ_STEP_LESS_TIMES;
				}
			}
			nCollectOK = 0;
		}
		else												// 没有足够的PCA0中断次数
		{
			fFreqWaken += FREQ_STEP_NO_SUCCEED;	
		}
	}

	// 循环测试频率结束	
	if(bFreqFind == 0) fFreqAver = 0;						// 没找到频率，返回无效频率

	//返回结果
	return fFreqAver;
}

/******************************************************************************
**名	称： PCA0Int()
**功	能： PCA中断响应函数
**入口参数： 无
**出口参数： 无
**说	明： 判断中断的类型，作不同的处理
******************************************************************************/
unsigned int  xdata nCollectValue[32];	// 记录捕捉值
unsigned char xdata nCollectOverV[32];	// 记录中断次数
float		  xdata fFreq[32];			// 存放频率
unsigned int  xdata nFreqN = 0;			// 扫频时每个频率需要输出的个数
unsigned char xdata nTestStatus = 0;	// 测试状态
													 
void PCA0Int() interrupt 11
{
	//进中断
	OSIntEnter();

	//激振处理*******************************************************************************
	if((INT_TYPE_WAKEN == nIntType) && CCF1)	// CEX1，激振
	{
		nFreq16Counter += nFreq16Span;

		// 重新加载电平变换需要的周期数
		PCA0CPL1 = LOBYTE(nFreq16Counter);
		PCA0CPH1 = HIBYTE(nFreq16Counter);

		//判断是否激振完毕		
		nWakenIndex++;
		if(nWakenIndex >= nWakenMax)
		{
			OSSemPost(pVWSEvent);
		} 	
	}

	//拾振处理*******************************************************************************
	if(INT_TYPE_COLLECT == nIntType) 
	{
		//捕捉有效
		if(CCF0 == 1)									
		{
			//判断拾振是否有效，如果无效，提前结束拾振
			if(P0&0x08 != 0x08)
			{
				DisablePCA0();						// 关CEX0中断
				DisableCFInt();						// 禁止PCA0溢出中断
				DisableCCF0Int();					// 禁止CEX0中断

				//无效标志
				nCollectOK = FALSE;

			   	//发送信号量
				OSSemPost(pVWSEvent);
			}
			else
			{ 				
				//记录每一次的捕捉值
				nCollectValue[nCollectIntCount]  = PCA0CPL0;
				nCollectValue[nCollectIntCount] += (PCA0CPH0 << 8);
				nCollectOverV[nCollectIntCount]  = nCollectOverCount;
		
				if(nCollectIntCount >= VWS_COLLECT_COUNT)	// 大于20次认为结束
				{
					// 实际测试频率的时候，在测到频率之后需要关闭PCA0计数器
					DisablePCA0();						// 关CEX0中断
					DisableCFInt();						// 禁止PCA0溢出中断
					DisableCCF0Int();					// 禁止CEX0中断
	
					nCollectIntCount = 0;				// 中断次数清零
					nSensorStatus = SENSOR_IDLE;		// 进入空闲状态
					nCollectOK = 1;						// 频率测试结束
	
					//发送信号量
					OSSemPost(pVWSEvent);
				}

				nCollectIntCount++;								//捕捉次数加1
			}
		}

		// PCA0溢出中断
		if(CF == 1)											
		{	
			// 拾振时，对溢出次数进行计数
			if(SENSOR_COLLECT == nSensorStatus)				// 如果在拾振状态下
			{
				nCollectOverCount++;
			}
		}
	}


	// 清除所有中断标志位
	PCA0CN &= 0x40;										

	//出中断
	OSIntExit();
}


