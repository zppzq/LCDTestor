/*
*********************************************************************************************************
*                                               ColBus
*                                               通信协议
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               版权所有
*
*
* 文件名 : ColBusSlaveAps.c
* 作者   : 杨承凯(kady1984@hotmail.com)
*********************************************************************************************************
*/

#include <string.h>
#include "..\includes.h"
#include "ColBusDef.h"
#include "ColBusSlaveMac.h"
#include "ColBusSlaveDescribe.h"
#include "..\User\PowerManager.h"
#include "..\User\DataManage.h"
#include "..\User\SyncTimer.h"
#include "..\User\DataAcq.h"

//定义设备型号
#define COLLECTOR_MODEL			201


//数据缓冲区
#define CB_BUFF_APS_SEND_MAX	256
#define CB_BUFF_APS_RECV_MAX	256
uint8 buffCBApsRecv[CB_BUFF_APS_RECV_MAX];
uint8 buffCBApsSend[CB_BUFF_APS_SEND_MAX];


//配置信息
uint8 nCltFunc;										//采集器功能码
uint8 nCltCfg;										//采集器配置字
float fCltK[4];										//临时参数

//状态信息
BOOL g_bPrepareEnable = FALSE;						//记录是否使用预采集
BOOL g_bLocalSetZero = FALSE;						//记录是否使用本地调零
BOOL g_nAllowedSample = FALSE;						//是否允许采样
BOOL g_bDisableDynamic = TRUE;						//禁止动态采集
BOOL g_bDyncEnd = FALSE;							//用于AD结束动态采集时控制一次数据存储

//同步定时器控制
uint32 nHostTime = 0;								//记录主机已跑时间，单位100us
uint32 nCycleTimeLeft = 0;							//记录本地应该更新时间的机器周期
																				
//同步数据采集信息
uint8 	nCbSyncCode;								//同步功能码   
uint32 	nRequireAddr = 0;							//动态采集时被呼叫的数据地址
uint16 	nHostReadBytes = 0;							//上级要读取的数据个数
uint32 	nHostDeviceByteDiff = 0;					//主机和本地的数据个数差值
uint32 nHostCount;									//远程存储计数值
float  fHostCount;									//远程存储计数值浮点数
uint16 nBrgRefSampleSplt = SAMPLE_SLIP_SYNC_RT;		//动态采集参考采样率(同步信号频率)
float  fBrgSampleDelt;								//主机参考索引乘以这个参数就等于采集次数

//校正时要用的参数
static float TmpFloat;
static float TmpFloatZero; 
static float fMesureCur;

//擦除数据的长度
static uint32 FlashEraseBlockCount;

//事件
static OS_EVENT* pCommDiskSem;

//外部变量引用
extern uint8 g_bTaskBusy;							//BUSY控制
extern uint8 g_nState;								//采集器状态字

extern float g_fADStoreParam[DATA_ACQ_COUNT];		//数据转换系数
extern float g_fMesureZeroVol[DATA_ACQ_COUNT];		//存储到存储器中的调零电压
extern DT_STORE g_sMesureZero[DATA_ACQ_COUNT];		//调零数据，用于计算的参数

extern float fBrgSetSampleRate;
extern uint16 nBrgSetSampleRate;					//要设置的采样率
extern uint32 nLocalCount;							//本地存储计数值
extern uint32 nDynLocalAddr[DATA_ACQ_COUNT];		//数据存储区地址长度
extern float g_fADStep[DATA_ACQ_COUNT];

//休眠参数设置
extern uint16	g_nSleepPoint;					    //多久之后休眠(单位为5S)
extern uint16	g_nSleepTicketMax;					//休眠时间为多久(单位为5S)

//初始化
void CBSlaveApsInit(void)
{
	//创建磁盘访问信号
	pCommDiskSem = OSSemCreate(0);

	//同步定时器初始化
	SyncTimerInit();
}

//有限状态处理
int16 CBSlaveApsFsm(void)
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	int16 nApsRecv;
	uint8 nCmd, nSubCmd;
	uint8 i, nEpIndex, nByteIndex;
	uint8* pAckBuff = buffCBApsSend;

	//ColBus设备使用“接收-应答”模式
	nApsRecv = CBSlaveNetRecv(buffCBApsRecv, CB_BUFF_APS_RECV_MAX);
	if(nApsRecv <= 0) return nApsRecv;

	//获取命令
	nCmd = buffCBApsRecv[0];
	pAckBuff[0] = nCmd;	//设置返回原指令
	
	//解析指令
	switch(nCmd)
	{
	//复位
	case CB_RESET:
		if((buffCBApsRecv[1] == 0xA5) && (buffCBApsRecv[1] == 0xA5))
		{
			//应答
			CBSlaveNetSend(pAckBuff, 1);
			
			//执行复位操作
			//...
		}
		else
		{
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_KEY;
			CBSlaveNetSend(pAckBuff, 2);			
		}
		break;

	//获取版本
	case CB_VERSION:
		pAckBuff[1] = nCBVersion[0];
		pAckBuff[2] = nCBVersion[1];
		CBSlaveNetSend(pAckBuff, 3);	
		break;

	//采集开始
	case CB_SAMPLE_START:
		pAckBuff[1] = buffCBApsRecv[1];
		if(buffCBApsRecv[1] == CB_OP_STATIC)
		{
			//设置状态
			g_nState |= COLLECTOR_STATIC;					//置静态采集标志位
			g_nState &= ~COLLECTOR_DYNAMIC;					//取消动态采集标志位

			//应答
			CBSlaveNetSend(pAckBuff, 2);   
		}
		else if(buffCBApsRecv[1] == CB_OP_STATICS)
		{
			nByteIndex = 0;
			pAckBuff[1] = buffCBApsRecv[1];	//设置子指令
			pAckBuff[2] = buffCBApsRecv[2];	//设置端点个数

			for(i = 0; i < buffCBApsRecv[2]; i++)
			{
				nEpIndex = buffCBApsRecv[3 + i];		//端点索引
				
				//防止端点溢出
				if(nEpIndex >= DEVICE_EP_COUNT)
				{
					pAckBuff[2]--;	//返回端点个数减一	
					continue;
				}
				
				//装载数据
				pAckBuff[3 + nByteIndex] = nEpIndex;
				nByteIndex++;	
			}

			//设置状态
			g_nState |= COLLECTOR_STATIC;					//置静态采集标志位
			g_nState &= ~COLLECTOR_DYNAMIC;					//取消动态采集标志位

			//应答
			CBSlaveNetSend(pAckBuff, 3 + nByteIndex); 

		}
		else if(buffCBApsRecv[1] == CB_OP_DYNAMIC)
		{
			//应答
			CBSlaveNetSend(pAckBuff, 2); 

			//使能数据采集
			if(g_bDisableDynamic == FALSE)
			{
				//设置状态
				g_nState &= ~COLLECTOR_STATIC;
				g_nState |= COLLECTOR_DYNAMIC;
	
				//计算采样周期
				fBrgSetSampleRate = ntohf(*((float*)(buffCBApsRecv+2)));
				nBrgRefSampleSplt = ntohl(*((uint32*)(buffCBApsRecv+6)));

				nBrgSetSampleRate = (uint16)fBrgSetSampleRate; 						
				fBrgSampleDelt = (float)nBrgSetSampleRate;
				fBrgSampleDelt *= (float)nBrgRefSampleSplt;
				fBrgSampleDelt /= 1000000.0f;
	
				nLocalCount = 0;						//索引字清零
				nHostCount = 0;							//主机索引清零
	
				//设置采样率
				SetSyncTimerRate(nBrgSetSampleRate);
				
				//更改状态
				ResetDataEraseBlock();
				
				//启动数据采集
				DataAcqStart();							//启动数据采集
	
				//开启定时器
				g_bDyncEnd = FALSE;						//取消结束标志
				g_nAllowedSample = FALSE;				//不允许采样(收到同步信号时才允许采样)
				SyncTimerResetRun();					//开启同步定时器
				OpenSyncTimerISR();						//关闭同步定时器
			}	
	
		}
		else
		{
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 2);		
		}
		break;

	//暂停
	case CB_SAMPLE_PAUSE:
		pAckBuff[1] = buffCBApsRecv[1];
		if(buffCBApsRecv[1] == CB_OP_DYNAMIC)
		{

			//回复当前索引
			*((uint32*)(pAckBuff+2)) = ntohl(nLocalCount);
		
			//应答
			CBSlaveNetSend(pAckBuff, 6); 

			g_nAllowedSample = 0;						//不允许采样
			SyncTimerStop();							//关同步定时器
			CloseSyncTimerISR();						//关同步定时器中断
			DataAcqStop();								//停止数据采集
		}
		else if(buffCBApsRecv[1] == CB_OP_SYNC)
		{
			//应答
			CBSlaveNetSend(pAckBuff, 1); 

			nHostCount = ntohl(*((uint32*)(buffCBApsRecv+2)));				//主机数据个数

			//存储数据==========================
			if(nLocalCount < nHostCount)
			{
				//存储数据
				PostSampleSem();			
			}
			
			//存储最后一段数据
			DataAcqDataStore();				
		}
		else
		{
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 2);		
		}
		break;

	//恢复暂停
	case CB_SAMPLE_RESUME:
		pAckBuff[1] = buffCBApsRecv[1];
		if(buffCBApsRecv[1] == CB_OP_DYNAMIC)
		{ 
			//应答
			CBSlaveNetSend(pAckBuff, 2); 		

			//恢复数据采集
			DataAcqPauseResume();

			//开启定时器
			g_bDyncEnd = FALSE;						//取消结束标志
			g_nAllowedSample = 0;					//不允许采样
			SyncTimerResetRun();					//开启同步定时器
			OpenSyncTimerISR();						//开启同步定时器中断  

		}
		else
		{
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 2);		
		}
		break;

	//采集停止
	case CB_SAMPLE_STOP:
		//记录子指令
		pAckBuff[1] = buffCBApsRecv[1];
		if(buffCBApsRecv[1] == CB_OP_STATIC)
		{
			//设置状态
			g_nState &= ~COLLECTOR_STATIC;

			//应答
			CBSlaveNetSend(pAckBuff, 2);
		}
		else if(buffCBApsRecv[1] == CB_OP_STATICS)
		{
			nByteIndex = 0;
			pAckBuff[1] = buffCBApsRecv[1];	//设置子指令
			pAckBuff[2] = buffCBApsRecv[2];	//设置端点个数

			for(i = 0; i < buffCBApsRecv[2]; i++)
			{
				nEpIndex = buffCBApsRecv[3 + i];		//端点索引
				
				//防止端点溢出
				if(nEpIndex >= DEVICE_EP_COUNT)
				{
					pAckBuff[2]--;	//返回端点个数减一	
					continue;
				}
				
				//装载数据
				pAckBuff[3 + nByteIndex] = nEpIndex;
				nByteIndex++;
				
			}

			//设置状态
			g_nState &= ~COLLECTOR_STATIC;					//取消静态采集标志位

			//回复数据
			CBSlaveNetSend(pAckBuff, 3 + nByteIndex); 

		}
		else if(buffCBApsRecv[1] == CB_OP_DYNAMIC)
		{
			//应答
			CBSlaveNetSend(pAckBuff, 2); 

			//设置状态
			g_nState &= ~COLLECTOR_DYNAMIC;

			//结束动态采集
			g_nAllowedSample = 0;						//不允许采样
			SyncTimerStop();							//关定时器3
			CloseSyncTimerISR();						//关定时器3中断
			DataAcqStop();								//关传感器

			//不能重复操作
			if(g_bDyncEnd == FALSE)
			{
				//设置已经停止采集
				g_bDyncEnd = TRUE;

				//禁止动态采集(只有擦除采集器了之后才会响应)
				g_bDisableDynamic = TRUE;					
				
				//存储最后一段数据
				DataAcqDataStore();
			}		
		}
		else
		{
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 2);		
		}
		break;

	//设备配置符
	case CB_DEVICE_CFG:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		case CB_DEVICE_MANUFACTURER:
			memcpy(pAckBuff+2, arrFactoryInfo, sizeof(arrFactoryInfo));
			CBSlaveNetSend(pAckBuff, 2 + sizeof(arrFactoryInfo));
			break;

		case CB_DEVICE_MODEL:
			memcpy(pAckBuff+2, arrDeviceInfo, sizeof(arrDeviceInfo));
			CBSlaveNetSend(pAckBuff, 2 + sizeof(arrDeviceInfo));
			break;

		case CB_DEVICE_EP_NUM:
			pAckBuff[2] = DEVICE_EP_COUNT;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		
		case CB_DEVICE_ENDIAN:
			pAckBuff[2] = DEVICE_ENDIAN_MODE;
			CBSlaveNetSend(pAckBuff, 3);
			break;

		default:
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}
		break;

	//设备参数
	case CB_DEVICE_PARA:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		case CB_DEVICE_PARA_NUM:
			pAckBuff[2] = 0;	//无需参数设置
			CBSlaveNetSend(pAckBuff, 3);
			break;

		default:
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}
		break;

	//端点配置符
	case CB_EP_CFG:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		//端点数据类型连续获取
		case CB_EP_TYPE_GETC:	 			
			//如果获取所有端点数据类型，则设置实际端点个数
			if(buffCBApsRecv[3] == 0xFF) buffCBApsRecv[3] = DEVICE_EP_COUNT;
			
			//检查参数是否在有效范围内
			if((buffCBApsRecv[2] < DEVICE_EP_COUNT) && (buffCBApsRecv[2] + buffCBApsRecv[3] <=DEVICE_EP_COUNT))
			{
				pAckBuff[2] = buffCBApsRecv[2];	//设置端点起始
				pAckBuff[3] = buffCBApsRecv[3]; //设置端点个数
				memcpy(pAckBuff + 4, arrEpType + buffCBApsRecv[2], buffCBApsRecv[3]);
				CBSlaveNetSend(pAckBuff, 4 + buffCBApsRecv[3]);
			}
			else
			{
				//异议应答
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//端点数据类型抽样获取
		case CB_EP_TYPE_GETS:
			if(buffCBApsRecv[2] <= DEVICE_EP_COUNT)
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//设置端点个数
				for(i = 0; i < buffCBApsRecv[2]; i++)
				{
					nEpIndex = buffCBApsRecv[3 + i];		//端点索引
					
					//防止端点溢出
					if(nEpIndex >= DEVICE_EP_COUNT)
					{
						pAckBuff[2]--;	//返回端点个数减一	
						continue;
					}
					
					//装载数据
					pAckBuff[3 + nByteIndex] = nEpIndex;
					nByteIndex++;

					pAckBuff[3 + nByteIndex] = arrEpType[nEpIndex];  //确保nEpIndex < DEVICE_EP_COUNT
					nByteIndex++;
				}

				CBSlaveNetSend(pAckBuff, 3 + nByteIndex);
			}
			else
			{
				//异议应答
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//端点属性连续获取
		case CB_EP_PROP_GETC:
			//如果获取所有端点属性，则设置实际端点个数
			if(buffCBApsRecv[3] == 0xFF) buffCBApsRecv[3] = DEVICE_EP_COUNT;
			
			//检查参数是否在有效范围内
			if((buffCBApsRecv[2] < DEVICE_EP_COUNT) && (buffCBApsRecv[2] + buffCBApsRecv[3] <=DEVICE_EP_COUNT))
			{
				pAckBuff[2] = buffCBApsRecv[2];	//设置端点起始
				pAckBuff[3] = buffCBApsRecv[3]; //设置端点个数
				memcpy(pAckBuff + 4, arrEpProp + buffCBApsRecv[2], buffCBApsRecv[3]);
				CBSlaveNetSend(pAckBuff, 4 + buffCBApsRecv[3]);
			}
			else
			{
				//异议应答
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//端点属性抽样获取
		case CB_EP_PROP_GETS:
			if(buffCBApsRecv[2] <= DEVICE_EP_COUNT)
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//设置端点个数
				for(i = 0; i < buffCBApsRecv[2]; i++)
				{
					nEpIndex = buffCBApsRecv[3 + i];		//端点索引
					
					//防止端点溢出
					if(nEpIndex >= DEVICE_EP_COUNT)
					{
						pAckBuff[2]--;	//返回端点个数减一	
						continue;
					}
					
					//装载数据
					pAckBuff[3 + nByteIndex] = nEpIndex;
					nByteIndex++;

					pAckBuff[3 + nByteIndex] = arrEpProp[nEpIndex];
					nByteIndex++;
				}

				CBSlaveNetSend(pAckBuff, 3 + nByteIndex);
			}
			else
			{
				//异议应答
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//端点物理单位连续获取
		case CB_EP_UNIT_GETC:
			//如果获取所有端点物理单位，则设置实际端点个数
			if(buffCBApsRecv[3] == 0xFF) buffCBApsRecv[3] = DEVICE_EP_COUNT;
			
			//检查参数是否在有效范围内
			if((buffCBApsRecv[2] < DEVICE_EP_COUNT) && (buffCBApsRecv[2] + buffCBApsRecv[3] <=DEVICE_EP_COUNT))
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//设置端点起始
				pAckBuff[3] = buffCBApsRecv[3]; //设置端点个数
				for(i = 0; i < buffCBApsRecv[3]; i++)
				{
					nEpIndex = buffCBApsRecv[2] + i;	//端点索引
					memcpy(pAckBuff + 4 + nByteIndex, arrEpUnitPt[nEpIndex], arrEpUnitLen[nEpIndex]);
					nByteIndex += arrEpUnitLen[nEpIndex];
				}
			
				CBSlaveNetSend(pAckBuff, 4 + nByteIndex);
			}
			else
			{
				//异议应答
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//端点物理单位抽样获取
		case CB_EP_UNIT_GETS:
			if(buffCBApsRecv[2] <= DEVICE_EP_COUNT)
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//设置端点个数
				for(i = 0; i < buffCBApsRecv[2]; i++)
				{
					nEpIndex = buffCBApsRecv[3 + i];		//端点索引
					
					//防止端点溢出
					if(nEpIndex >= DEVICE_EP_COUNT)
					{
						pAckBuff[2]--;	//返回端点个数减一	
						continue;
					}
					
					//装载数据
					pAckBuff[3 + nByteIndex] = nEpIndex;
					nByteIndex++;
					
					memcpy(pAckBuff + 3 + nByteIndex, arrEpUnitPt[nEpIndex], arrEpUnitLen[nEpIndex]);
					nByteIndex += arrEpUnitLen[nEpIndex];
				}

				CBSlaveNetSend(pAckBuff, 3 + nByteIndex);
			}

			else
			{
				//异议应答
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		case CB_EP_PARA_SETC:
			nEpIndex = buffCBApsRecv[2];
			if(nEpIndex < DEVICE_EP_COUNT)
			{
				//校验设备类型
				if(COLLECTOR_MODEL == ntohs(*((uint16*)(buffCBApsRecv+3))))
				{
					nCltFunc = *(buffCBApsRecv+5);						//功能号
					nCltCfg = *(buffCBApsRecv+6);						//配置字
	
					//预采集标志
					if(0 < (nCltCfg & 0x02)) g_bPrepareEnable = TRUE;
					else g_bPrepareEnable = FALSE;						
			
					//扫频范围 
					fCltK[0] = ntohf(*((float*)(buffCBApsRecv+7)));			//量程
					fCltK[1] = ntohf(*((float*)(buffCBApsRecv+11)));		//灵敏度系数(K)
					fCltK[2] = ntohf(*((float*)(buffCBApsRecv+15)));		//电池最低电压
	
					//电压率设置
					if(fCltK[2] > 3.6f)
					{
						SetPowerRange(fCltK[2], fCltK[2] + 1.0f);
					}
	
					//设置参数
					OnHostParamSet(buffCBApsRecv[2], nCltFunc, fCltK[1], fCltK[0]);
	
					if(nCltCfg & 0x01) 
					{
						//初始化本地调零值
						for(i=0;i<DATA_ACQ_COUNT;i++)
						{
							g_sMesureZero[i] = (DT_STORE)(g_fMesureZeroVol[i] * g_fADStoreParam[i]);
						}
						
						//调零标志置位
						g_bLocalSetZero = TRUE;
					}
					else 
					{
						//清除调零数值
						DataInitZero(g_sMesureZero);
						
						//调零标志复位
						g_bLocalSetZero = FALSE;
					}
	
					
					//回复
					pAckBuff[2] = buffCBApsRecv[2];
					*((uint16*)(pAckBuff+3)) = (uint16)COLLECTOR_MODEL;
					pAckBuff[5] = GetPowerRate();
	
					CBSlaveNetSend(pAckBuff, 6);  
				}
	
				//采集器类型错误
				else
				{
					pAckBuff[0] |= 0x80;
					pAckBuff[2] = CB_ERR_MODEL;
					CBSlaveNetSend(pAckBuff, 3);
				}
			}
			else
			{
				//端点地址不正确
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}

			break;

		case CB_EP_CALI_SETC:

			//校验关键字
			if(ntohl(*((uint32*)(buffCBApsRecv+2))) != BRG_CALI_KEY)
			{
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_KEY;
				CBSlaveNetSend(pAckBuff, 3);			
			}
			else
			{
				nEpIndex = 	buffCBApsRecv[6];

				if(nEpIndex < DEVICE_EP_COUNT)
				{
					//回复先
					CBSlaveNetSend(pAckBuff, 2);

					//校正量
					TmpFloat = ntohf(*((float*)(buffCBApsRecv+8)));

					//(步进*10000)/0到10000的电压差值
					g_fADStep[nEpIndex] = TmpFloat;

					//存储校正值
					DataAcqParamStore();
					ParamStore();

					//重新计算AD系数
					DataAcqParamCacul();
				}
				else
				{
					pAckBuff[0] |= 0x80;
					pAckBuff[2] = CB_ERR_EP_ADDR;
					CBSlaveNetSend(pAckBuff, 3);
				}
			}
			break;

		case CB_EP_CALI_REFC:

			//校验关键字
			if(ntohl(*((uint32*)(buffCBApsRecv+2))) != BRG_CALI_KEY)
			{
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_KEY;
				CBSlaveNetSend(pAckBuff, 3);			
			}
			else
			{
				nEpIndex = 	buffCBApsRecv[6];

				if(nEpIndex < DEVICE_EP_COUNT)
				{
					//回复先
					CBSlaveNetSend(pAckBuff, 2);

					//校正量
					TmpFloat = ntohf(*((float*)(buffCBApsRecv+8)));
					TmpFloat = TmpFloat * 1.65f ;
					
					//零点		   
					TmpFloatZero= ntohf(*((float*)(buffCBApsRecv+12)));
					if(g_bLocalSetZero == TRUE)
					{
						//获取本地调零值
						TmpFloatZero = g_fMesureZeroVol[nEpIndex];	
					}
					
					//获取当前测量电压值
					fMesureCur = DataAcqGetStatic(nEpIndex);
					
	
					//若当前电压值与零点的差值
					if((fMesureCur - TmpFloatZero) == 0) fMesureCur = 0.0001f;
					else fMesureCur = (float)(fMesureCur -TmpFloatZero);
					
					//(步进*10000)/0到10000的电压差值
					g_fADStep[nEpIndex] = (g_fADStep[nEpIndex] * TmpFloat) / fMesureCur;

					//存储校正值
					DataAcqParamStore();
					ParamStore();

					//重新计算AD系数
					DataAcqParamCacul();
				}
				else
				{
					pAckBuff[0] |= 0x80;
					pAckBuff[2] = CB_ERR_EP_ADDR;
					CBSlaveNetSend(pAckBuff, 3);
				}
			}
			break;

		case CB_EP_CALI_RSTC:

			//校验关键字
			if(ntohl(*((uint32*)(buffCBApsRecv+2))) != BRG_CALI_KEY)
			{
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_KEY;
				CBSlaveNetSend(pAckBuff, 3);			
			}
			else
			{
				nEpIndex = 	buffCBApsRecv[6];

				if(nEpIndex < DEVICE_EP_COUNT)
				{
					//回复先
					CBSlaveNetSend(pAckBuff, 2);

					//复位到默认值
					g_fADStep[nEpIndex] = F_AD_STEP;

					//存储校正值
					DataAcqParamStore();
					ParamStore();

					//重新计算AD系数
					DataAcqParamCacul();
				}
				else
				{
					pAckBuff[0] |= 0x80;
					pAckBuff[2] = CB_ERR_EP_ADDR;
					CBSlaveNetSend(pAckBuff, 3);
				}
			}
			break;

		default:
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}
		break;

	//端点数据抽样获取
	case CB_EP_DATA_PREPS:
		
		//是否忙
		if(g_bTaskBusy == TRUE)
		{
			//回复忙状态
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_BUSY;
			CBSlaveNetSend(pAckBuff, 2);
			break;
		}
		
		//处理
		if(buffCBApsRecv[1] <= DEVICE_EP_COUNT)
		{
			nByteIndex = 0;
			pAckBuff[1] = buffCBApsRecv[1];	//设置端点个数
			for(i = 0; i < buffCBApsRecv[1]; i++)
			{
				nEpIndex = buffCBApsRecv[2 + i];		//端点索引
				
				//防止端点溢出
				if(nEpIndex >= DEVICE_EP_COUNT)
				{
					pAckBuff[1]--;	//返回端点个数减一	
					continue;
				}
				
				//装载数据
				pAckBuff[2 + nByteIndex] = nEpIndex;
				nByteIndex++;  
			}

			//设置状态
			g_bTaskBusy =  TRUE;
			g_nState |= COLLECTOR_STATIC;					//置静态采集标志位
			g_nState &= ~COLLECTOR_DYNAMIC;					//取消动态采集标志位

			//启动数据采集
			DataAcqStart();

			//回复数据
			CBSlaveNetSend(pAckBuff, 2 + nByteIndex);
		}
		else
		{
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_EP_ADDR;
			CBSlaveNetSend(pAckBuff, 2);
		}
		break;

	//端点数据抽样获取
	case CB_EP_DATA_GETS:
		
		//是否忙
		if(g_bTaskBusy == TRUE)
		{
			//回复忙状态
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_BUSY;
			CBSlaveNetSend(pAckBuff, 2);
			break;
		}

		//处理数据
		if(buffCBApsRecv[1] <= DEVICE_EP_COUNT)
		{
			nByteIndex = 0;
			pAckBuff[1] = buffCBApsRecv[1];	//设置端点个数
			for(i = 0; i < buffCBApsRecv[1]; i++)
			{
				nEpIndex = buffCBApsRecv[2 + i];		//端点索引
				
				//防止端点溢出
				if(nEpIndex >= DEVICE_EP_COUNT)
				{
					pAckBuff[1]--;	//返回端点个数减一	
					continue;
				}
				
				//装载数据
				pAckBuff[2 + nByteIndex] = nEpIndex;
				nByteIndex++;
				
				//保护数据此时不被其他线程修改
				CB_ENTER_CRITICAL();
				g_fEpValue[nEpIndex] = (DT_STORE)((DataAcqGetStatic(nEpIndex) - g_fMesureZeroVol[nEpIndex]) * g_fADStoreParam[nEpIndex]);
				g_fEpValue[nEpIndex] = htons(g_fEpValue[nEpIndex]);
				memcpy(pAckBuff + 2 + nByteIndex, arrEpDataPt[nEpIndex], arrEpDataLen[nEpIndex]);
				CB_EXIT_CRITICAL();
				
				nByteIndex += arrEpDataLen[nEpIndex];
			}

			CBSlaveNetSend(pAckBuff, 2 + nByteIndex);
		}
		else
		{
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_EP_ADDR;
			CBSlaveNetSend(pAckBuff, 2);
		}
		break;

	//调零操作
	case CB_ZERO:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		//连接测试
		case CB_ZERO_SETS:

			//是否忙
			if(g_bTaskBusy == TRUE)
			{
				//回复忙状态
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_BUSY;
				CBSlaveNetSend(pAckBuff, 3);
				break;
			}
			
			//处理
			if(buffCBApsRecv[2] <= DEVICE_EP_COUNT)
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//设置端点个数
				for(i = 0; i < buffCBApsRecv[2]; i++)
				{
					nEpIndex = buffCBApsRecv[3 + i];		//端点索引
					
					//防止端点溢出
					if(nEpIndex >= DEVICE_EP_COUNT)
					{
						pAckBuff[2]--;	//返回端点个数减一	
						continue;
					}
					
					//装载数据
					pAckBuff[3 + nByteIndex] = nEpIndex;
					nByteIndex++;
					
					g_fMesureZeroVol[nEpIndex] = DataAcqGetStatic(nEpIndex);
					g_sMesureZero[nEpIndex] = (DT_STORE)(g_fMesureZeroVol[nEpIndex] * g_fADStoreParam[nEpIndex]);
				}
	
				//回复数据
				CBSlaveNetSend(pAckBuff, 3 + nByteIndex);

				//存储零点
				DataAcqParamStore();
				ParamStore();
			}
			else
			{
				//异议应答
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//从设备通信缓冲区长度
		case CB_ZERO_CANCELS:
			
			//是否忙
			if(g_bTaskBusy == TRUE)
			{
				//回复忙状态
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_BUSY;
				CBSlaveNetSend(pAckBuff, 3);
				break;
			}
			
			//处理
			if(buffCBApsRecv[2] <= DEVICE_EP_COUNT)
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//设置端点个数
				for(i = 0; i < buffCBApsRecv[2]; i++)
				{
					nEpIndex = buffCBApsRecv[3 + i];		//端点索引
					
					//防止端点溢出
					if(nEpIndex >= DEVICE_EP_COUNT)
					{
						pAckBuff[2]--;	//返回端点个数减一	
						continue;
					}
					
					//装载数据
					pAckBuff[3 + nByteIndex] = nEpIndex;
					nByteIndex++;
					
					g_fMesureZeroVol[nEpIndex] = 0;
					g_sMesureZero[nEpIndex] = 0;
				}
	
				//回复数据
				CBSlaveNetSend(pAckBuff, 3 + nByteIndex);

				//存储零点
				DataAcqParamStore();
				ParamStore();
			}
			else
			{
				//异议应答
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		default:
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}
		break;

	//存储访问
	case CB_EP_MEM:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		//连接测试
		case CB_EP_MEM_SEG_ERASE:
			
			//设置状态
			SyncTimerStop();
			
			//停止传感器以防止异常
			DataAcqStop();

			//应答先
			CBSlaveNetSend(pAckBuff, 2);

			//等等让数据采集线程停止
			OSTimeDly(3);

			//获取擦除长度
			FlashEraseBlockCount = ntohl(*((uint32*)(buffCBApsRecv+11)));

			//发送数据擦除事件
			PostDataEraseEvent(FlashEraseBlockCount);  
			
			//设置状态
			g_bDisableDynamic = FALSE;			
					
			break;

		case CB_EP_MEM_SEG_DATA:
			nEpIndex = buffCBApsRecv[2];

			if(nEpIndex < DEVICE_EP_COUNT)
			{
				nRequireAddr = ntohl(*((uint32*)(buffCBApsRecv+3)));				//获取主机数据索引 
				nHostReadBytes = ntohs(*((uint32*)(buffCBApsRecv+7)));	
			
	
				//设置回复信息
				memcpy(pAckBuff + 2, buffCBApsRecv + 2, 7);	//ep, nRequireAddr, nHostReadBytes
				//MemDataLoad(pCommDiskSem, pAckBuff+9, nRequireAddr, nHostReadBytes, nEpIndex);   //MemLoad在结束采集后立刻读可能出现异常
				DataLoad(pCommDiskSem, pAckBuff+9, nRequireAddr, nHostReadBytes, nEpIndex);
	
				//回复
				CBSlaveNetSend(pAckBuff, 9 + nHostReadBytes);
			}
			else
			{
				//异议应答
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}

			break;

		default:
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}

		break;

	//动态同步
	case CB_SAMPLE_SYNC:
		//是否已经开始采集，如果没有则退出=====================================			
		if((g_nState & COLLECTOR_DYNAMIC) != COLLECTOR_DYNAMIC) return FALSE;

		//判断是否被禁止=======================================================
		if(g_bDisableDynamic == TRUE) return FALSE;

		//允许采样
		g_nAllowedSample = TRUE;

		//获取功能字===========================================================
		nCbSyncCode = buffCBApsRecv[1];
		
		//同步和采样数据=======================================================
		if((nCbSyncCode & CB_SYNC_CALI_TIME) != 0 )
		{
			//主机的索引
			nHostTime = ntohl(*((uint32*)(buffCBApsRecv+2)));				//获取主机数据索引

			//计算主机次数
			fHostCount = (float)nHostTime;						  	//数据索引(浮点数运算)
			fHostCount *= fBrgSampleDelt;						   	//计算采样索引
			nHostCount = (uint32)fHostCount;						//采样索引(整型)

			//计算空余的时间
			fHostCount = (float)nHostCount;							//当前采集个数
			fHostCount /= fBrgSampleDelt;							//当前采集个数对应的时间(浮点数运算)
			nCycleTimeLeft = (uint32)fHostCount;					//当前采集个数对应的时间(取整)
			nCycleTimeLeft = nHostTime - nCycleTimeLeft;			//空余的时间(100us单位)
			
			//校准定时器========================
			//关定时器3中断以防止异常
			CloseSyncTimerISR();	
			
			//定时器3停止
			SyncTimerStop();

			//校准定时器3
			//如果采集器快很多
			if(nLocalCount > nHostCount + 1)
			{
				SyncTimerResetRun();
			}
			else
			{
				//快一点或慢一些
				SyncTimerCaliAndRun();
			}

			//存储数据==========================
			if(nLocalCount < nHostCount)
			{
				//存储数据
				PostSampleSem();			
			}

			//开定时器3中断
			OpenSyncTimerISR();					
		}

		//是否要回传动态实时数据=====================================================
		if(CheckAddrNet(buffCBApsRecv+7, buffCBApsRecv[6]) == TRUE)
		{
			//选择了当前采集器
			//回传空数据
			if((nCbSyncCode & SYNC_RETURN_EMPTY) != 0x0000 )
			{
				pAckBuff[0] = BRG_REASON_REPORT;	
				pAckBuff[1] = BRG_REASON_EMPTY;
				CBSlaveNetSend(pAckBuff, 2);
				break;
			} 

			//解析通道号
			nEpIndex = buffCBApsRecv[7 + buffCBApsRecv[6]];

			//需要回传数据地址
			nRequireAddr = ntohl(*(uint32*)(buffCBApsRecv + 8 + buffCBApsRecv[6]));

			//要读取的数据长度
			nHostReadBytes = ntohs(*(uint16*)(buffCBApsRecv + 12 + buffCBApsRecv[6]));
		
			//关定时器3中断以防止异常
			CloseSyncTimerISR();		
			
			//计算要回传的数据长度
			OS_ENTER_CRITICAL();
			if(nDynLocalAddr[nEpIndex] > nRequireAddr)
			{
				nHostDeviceByteDiff = nDynLocalAddr[nEpIndex] - nRequireAddr;
				if(nHostDeviceByteDiff > nHostReadBytes)
				{
					nHostDeviceByteDiff = nHostReadBytes;
				}
			}
			else nHostDeviceByteDiff = 0;
			OS_EXIT_CRITICAL();

			//回复信息
			pAckBuff[0] = CB_SAMPLE_SYNC;
			pAckBuff[1] = nEpIndex;			
			pAckBuff[2] = nRequireAddr;
			
			//装载数据
			nHostReadBytes = nHostDeviceByteDiff;
			*(uint16*)(pAckBuff + 3) = ntohs(nHostReadBytes);

			//读数据
			DataLoad(pCommDiskSem, pAckBuff + 5, nRequireAddr, nHostReadBytes, nEpIndex);
			
			//开定时器3中断
			OpenSyncTimerISR();

			//返回数据
			CBSlaveNetSend(pAckBuff, 5 + nHostReadBytes);	
		}
			
		//退出
		break;
							 
	//通信描述符
	case CB_COMM_CFG:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		//连接测试
		case CB_COMM_CHECK:
			CBSlaveNetSend(pAckBuff, 2);
			break;

		//获取通信缓冲区长度
		case CB_COMM_BUFFLEN_GET:
			*(uint16*)(pAckBuff+2) = ntohs(CB_BUFF_APS_RECV_MAX);
			*(uint16*)(pAckBuff+4) = ntohs(CB_BUFF_APS_SEND_MAX);
			CBSlaveNetSend(pAckBuff, 6);
			break;

		//通信质量校验
		case CB_COMM_QUALITY_TEST:
			memcpy(pAckBuff, buffCBApsRecv, nApsRecv);
			CBSlaveNetSend(pAckBuff, nApsRecv);
			break;

		//休眠时间设置
		case CB_COMM_BEAT_SET:
			//校验关键码
			if(ntohl(*(uint32*)(buffCBApsRecv+2)) == BEAT_SET_KEY)
			{
				//下面两个变量单位都为5秒，上位机要将时间换算一下先
				g_nSleepPoint = ntohs(*(uint16*)(buffCBApsRecv+6));
				g_nSleepTicketMax = ntohs(*(uint16*)(buffCBApsRecv+8));

				//回复
				CBSlaveNetSend(pAckBuff, 2);
			}
			break;

		default:
			//异议应答
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}
		break;

	//通信唤醒
	case CB_COMM_WAKE:
		//校验关键码
		if(ntohs(*(uint16*)(buffCBApsRecv+1)) == WAKEUP_RESET_KEY)
		{
			//校验成功
			return 1;
		}	
		else
		{	//校验失败
			return 0;
		}
		//break;	//已经return了

	default:
		//异议应答
		pAckBuff[0] |= 0x80;
		pAckBuff[1] = CB_ERR_CMD_US;
		CBSlaveNetSend(pAckBuff, 2);
		break;
	}	 

	return TRUE;
}






