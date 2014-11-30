/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: UsbComm.c
**创   建   人: 杨承凯(kady1984@163.com)
**创 建 日  期: 2008年04月17日
**最后修改日期: 2008年04月17日
**描        述: USB中断处理相关函数
*****************************************************************************************************************/
#include <stdio.h>
#include "UsbInclude.h"
#include "c8051f340.h"
#include "F34xUsbRegister.h"
#include "UsbStandDefine.h"
#include "UsbDescriptor.h"
#include "..\OsCommon\Includes.h"

//常量定义
BYTE code USB_ONES_PACKET[2] = {0x01, 0x00}; 		//USB DATA1数据包
BYTE code USB_ZERO_PACKET[2] = {0x00, 0x00}; 		//USB0 DATA0数据包

//内部全局变量定义
static xdata CUsbSetupCmd Setup; 									//设备请求缓冲区
static xdata BYTE UsbState; 										//记录当前USB的状态
static xdata unsigned int UsbDataSize; 								//数据长度
static xdata unsigned int UsbDataSent; 								//已经发送过的数据
static xdata BYTE* UsbDataPtr; 										//数据指针
static xdata BYTE UsbEpStatus[3] = { EP_IDLE, EP_IDLE, EP_IDLE };	//每个端点的状态

//OUT数据缓冲区
unsigned xdata UsbOutCount;								
unsigned char xdata UsbOutBuffer[EP2_PACKET_SIZE];

//IN数据缓冲区
unsigned xdata bUsbInNeed;
unsigned xdata UsbInCount;
unsigned char xdata UsbInBuffer[EP1_PACKET_SIZE];

//信号量定义
OS_EVENT *pUsbRecvEvent;
OS_EVENT *pUsbSendEvent;
unsigned char UsbErr;


//函数定义========================================================================================================

/*****************************************************************************************************************
* 名	称：Usb0Init()
* 功	能：USB0初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void Usb0Init(void) reentrant
{
	UsbWriteByte(POWER, 0x08); 			//复位USB
	UsbWriteByte(IN1IE, 0x07); 			//使能USB端点0，1，2的输入中断
	UsbWriteByte(OUT1IE,0x07); 			//使能USB端点0，1，2的输出中断
	UsbWriteByte(CMIE,  0x07); 			//使能USB复位，暂停，恢复的中断
	EIE1 |= 0x02; 						//使能USB总中断
}

/*****************************************************************************************************************
* 名	称：Usb0Open()
* 功	能：开启USB0
* 入口参数：无
* 出口参数：无
* 说	明：要在启动任务的一开始调用，否则无法及时回复主机的连接信号
*****************************************************************************************************************/
void Usb0Open() reentrant
{
	pUsbRecvEvent = OSSemCreate(0);		//创建接收数据的信号量
	pUsbSendEvent = OSSemCreate(0);		//创建发送数据的信号量

	USB0XCN = 0xE0; 					//使能USB端口驱动器，全速模式
	UsbWriteByte(CLKREC, 0x80); 		//使能时钟恢复
	UsbWriteByte(POWER, 0x01); 			//开启USB，同时使能总线挂起检测
}

/*****************************************************************************************************************
* 名	称：Usb0VariInit()
* 功	能：USB0变量初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void Usb0VariInit(void) reentrant
{
	UsbInCount=0;
	UsbOutCount=0;
	bUsbInNeed = FALSE;

	pUsbRecvEvent = NULL;
	pUsbSendEvent = NULL;
}

/*****************************************************************************************************************
* 名	称：UsbFifoRead()
* 功	能：从FIFO队列读取数据
* 入口参数：nAddr - 端点名称；pDat - 接收缓冲区指针；nLen - 数据个数
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void UsbFifoRead(BYTE nAddr, BYTE *pDat, unsigned int nLen) reentrant
{
	int i;

	EA = 0;

	if (nLen > 0)
	{
		while(USB0ADR & 0x80);				//等待直到空闲
		USB0ADR = nAddr; 					//设置地址
		USB0ADR |= 0xC0; 					//设置自动初始化数据读操作
  
		//读出数据
		for (i=0; i<nLen; i++)
		{
			while(USB0ADR & 0x80);			//等待数据准备好
			pDat[i] = USB0DAT; 				//复制数据到输出缓冲区
		}

		USB0ADR = 0;						//数据读取完毕，清除自动初始化数据读操作标志
	}

	EA = 1;
}

/*****************************************************************************************************************
* 名	称：UsbFifoWrite()
* 功	能：向FIFO队列写入数据
* 入口参数：nAddr - 端点名称；pDat - 接收缓冲区指针；nLen - 数据个数
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void UsbFifoWrite(BYTE nAddr, BYTE *pDat, unsigned int nLen) reentrant
{
	int i;

	EA = 0;

	if (nLen > 0)
	{
		while(USB0ADR & 0x80);				//等待直到空闲
		USB0ADR = nAddr; 					//设置地址
  
		//读出数据
		for (i=0; i<nLen; i++)
		{
			while(USB0ADR & 0x80);			//等待数据准备好
			USB0DAT = pDat[i]; 				//复制数据到FIFO队列
		}
	}

	EA = 1;
}

/*****************************************************************************************************************
* 名	称：Usb0DeviceRecv()
* 功	能：USB0设备端接收函数
* 入口参数：pBuff - 数据区指针，nLen - 需要接收的数据个数，nTimeOut - 接收超时时间
* 出口参数：实际接收到的数据个数
* 说	明：无
*****************************************************************************************************************/
#ifdef _USB0_DEVICE_RECV_
int Usb0DeviceRecv(char* pBuff, int nLen, unsigned int nTimeOut) reentrant
{
	unsigned int nRealLen = 0;
	
	//等待接收事件
	OSSemPend(pUsbRecvEvent, nTimeOut, &UsbErr);
	
	if(UsbErr == OS_NO_ERR)
	{
		OS_ENTER_CRITICAL();

#ifdef USB_USE_MEM_BUFFERD		//如果使用了内存缓冲方式
		nRealLen = (nLen > UsbOutCount) ? UsbOutCount : nLen;						//确定有效数据长度
		memcpy(pBuff, UsbOutBuffer, nRealLen);										//复制数据
		UsbOutCount = 0;															

#else	//如果没有使用内存缓冲
		
		//选择端点2
		UsbWriteByte(INDEX, EP2_OUT_IDX);
		
		//读取数据到用户缓冲区
		nRealLen = (nLen > UsbOutCount) ? UsbOutCount : nLen;						//确定有效数据长度
		UsbFifoRead(FIFO_EP2, pBuff, nRealLen); 									//读取到用户数据区

		//将未读取完的数据读取完，以免下次读数据时出错
		if(UsbOutCount - nRealLen > 0)
		{
			//为了避免用户使用的pBuff就是UsbOutBuffer而出错，所以将UsbOutBuffer指针加上nRealLen
			UsbFifoRead(FIFO_EP2, UsbOutBuffer + nRealLen, UsbOutCount - nRealLen); //读取到内存	
		}

		//向主机发送读取确认
		UsbWriteByte(EOUTCSR1, 0);
		UsbOutCount = 0;
#endif

		OS_EXIT_CRITICAL();
	}

	return nRealLen;
}
#endif

/*****************************************************************************************************************
* 名	称：Usb0DeviceSend()
* 功	能：USB0设备端接收函数
* 入口参数：pBuff - 数据区指针，nLen - 需要发送的数据个数，nTimeOut - 发送超时时间
* 出口参数：实际发送的数据个数
* 说	明：无
*****************************************************************************************************************/
#ifdef _USB0_DEVICE_SEND_
int Usb0DeviceSend(char* pBuff, int nLen, unsigned int nTimeOut) reentrant
{
	unsigned int nRealLen = 0;

	//进入临界区
	OS_ENTER_CRITICAL();

	//判断以前的数据是否已经发送
	if(bUsbInNeed == TRUE)
	{
		OS_EXIT_CRITICAL();
		return -1;	
	}

	//向数据缓冲区写入数据	
	nRealLen = (nLen > EP1_PACKET_SIZE) ? EP1_PACKET_SIZE : nLen;
	UsbWriteByte(INDEX, EP1_IN_IDX); 			//设置端点1访问
	UsbFifoWrite(FIFO_EP1, pBuff, nRealLen);	//向In FIFO写入数据
	UsbWriteByte(EINCSR1, rbInINPRDY);			//设置数据准备好标志
	bUsbInNeed = TRUE;							//设置数据输入标志

	//退出临界区
	OS_EXIT_CRITICAL();

	//等待发送完事件
	OSSemPend(pUsbSendEvent, nTimeOut, &UsbErr);

	//返回成功或失败信息
	if(UsbErr == OS_NO_ERR) return nRealLen;
	return 0;
}
#endif

/*****************************************************************************************************************
* 名	称：UsbForceStall()
* 功	能：强制终止USB端点0通信
* 入口参数：无
* 出口参数：无
* 说	明：通信协议不符或通信异常时，强制停止通信
*****************************************************************************************************************/
void UsbForceStall(void) reentrant
{
	UsbWriteByte(INDEX, EP0_IDX);			//选择端点0
	UsbWriteByte(E0CSR, rbSDSTL); 			//设置强制停止位
	UsbEpStatus[0] = EP_STALL; 				//将端点0设置为强制停止状态
}

/*****************************************************************************************************************
* 名	称：UsbHandleIn()
* 功	能：USB In事务处理
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/ 
void UsbHandleIn() reentrant
{
	BYTE control_reg;

	UsbWriteByte(INDEX, EP1_IN_IDX); 			//设置端点1访问
	UsbReadByte(EINCSR1, control_reg); 			//读取端点1的控制寄存器

	if (UsbEpStatus[1] == EP_HALT) 				//如果端点0被挂起，则强行退出中断
	{
		UsbWriteByte(EINCSR1, rbInSDSTL);
	}

	//向主机发送数据
	else
	{
		if(control_reg & rbInSTSTL) 			//如果上次数据发送未成功，清除标志
		{
			UsbWriteByte(EINCSR1, rbInCLRDT);
		}

		if(control_reg & rbInUNDRUN) 			//如果上次数据没有返回信号，清除标志
		{
			UsbWriteByte(EINCSR1, 0x00);
		}

		//发送数据已发送信号
		if(bUsbInNeed == TRUE)
		{
			bUsbInNeed = FALSE;
			OSSemPost(pUsbSendEvent);
		}
	}
}

/*****************************************************************************************************************
* 名	称：UsbHandleOut()
* 功	能：USB数据接收处理函数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void UsbHandleOut() reentrant
{
	BYTE count=0;
	BYTE control_reg;

	UsbWriteByte(INDEX, EP2_OUT_IDX);
	UsbReadByte(EOUTCSR1, control_reg);

	if (UsbEpStatus[2] == EP_HALT)
	{
		UsbWriteByte(EOUTCSR1, rbOutSDSTL);		//状态错误，强行终止通信
	}

	//读取数据
	else
	{
		if (control_reg & rbOutSTSTL)
		{
			UsbWriteByte(EOUTCSR1, rbOutCLRDT);
		}

		//读取收到的数据长度
		UsbReadByte(EOUTCNTL, count);
		UsbOutCount = count;
		UsbReadByte(EOUTCNTH, count);
		UsbOutCount |= ((unsigned)count)<<8;

		//检查数据长度
		if(UsbOutCount > EP2_PACKET_SIZE) UsbOutCount = EP2_PACKET_SIZE;

#ifdef USB_USE_MEM_BUFFERD
		//如果使用内存缓冲方式，则读取到内存
		UsbFifoRead(FIFO_EP2, UsbOutBuffer, UsbOutCount); 		//读取到内存
		UsbWriteByte(EOUTCSR1, 0);								//向主机发送读取确认
#endif
		//发送收到数据信号
		OSSemPost(pUsbRecvEvent);
	}
}

/*****************************************************************************************************************
* 名	称：UsbGetStatus()
* 功	能：USB状态获取函数
* 入口参数：无
* 出口参数：无
* 说	明：USB标准请求函数
*****************************************************************************************************************/
void UsbGetStatus(void) reentrant
{
	//判断字段是否有效
	if (Setup.wValue.c[MSB] || Setup.wValue.c[LSB] || Setup.wLength.c[MSB] || (Setup.wLength.c[LSB] != 2))
	{
		UsbForceStall();
	}

	switch (Setup.bmRequestType)
	{
		//被请求者为设备
		case OUT_DEVICE:
			if (Setup.wIndex.c[MSB] || Setup.wIndex.c[LSB])
			{
				UsbForceStall(); 	//请求无效，停止通信
			}
			else
			{
				//回复两字节0x00
				UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
				UsbDataSize = 2; 
			}
			break;
	
		//被请求者为接口
		case OUT_INTERFACE:
			if ((UsbState != DEV_CONFIGURED) || Setup.wIndex.c[MSB] || Setup.wIndex.c[LSB])
			{
				UsbForceStall(); 	//请求无效，停止通信
			}
			else
			{
				//回复两字节0x00
				UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
				UsbDataSize = 2;
			}
			break;
	
		//被请求者为端点
		case OUT_ENDPOINT:
			if ((UsbState != DEV_CONFIGURED) || Setup.wIndex.c[MSB])
			{
				UsbForceStall();  	//请求无效，停止通信
			}
			else
			{
				//如果指向端点1
				if (Setup.wIndex.c[LSB] == IN_EP1)
				{
					if (UsbEpStatus[1] == EP_HALT)
					{
					 	//如果端点1是待机状态，回复0x01,0x00
						UsbDataPtr = (BYTE*)&USB_ONES_PACKET;
						UsbDataSize = 2;
					}
					else
					{
						//否则就回复0x00,0x00以指示端点已激活
						UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
						UsbDataSize = 2;
					}
				}
				else	//如果指向了另外的端点
				{
					//如果指向端点2
					if (Setup.wIndex.c[LSB] == OUT_EP2)
					{
						//如果端点2是待机状态，回复0x01,0x00
						if (UsbEpStatus[2] == EP_HALT)
						{
							UsbDataPtr = (BYTE*)&USB_ONES_PACKET;
							UsbDataSize = 2;
						}
						else
						{
							//否则就回复0x00,0x00以指示端点已激活
							UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
							UsbDataSize = 2;
						}
					}
					else
					{
						UsbForceStall(); 	//请求无效，停止通信
					}
				}
			}
			break;
	
		//无效的被请求者
		default:
			UsbForceStall();
			break;
	}

	//如果通信正常进行(没有调用过UsbForceStall()函数)
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, rbSOPRDY); 	//响应服务，硬件会清零OPRDY位                  
		UsbEpStatus[0] = EP_TX; 			//将端点零状态置为发送状态
		UsbDataSent = 0;					//将发送索引清零
	}
}

/*****************************************************************************************************************
* 名	称：UsbClearFeature()
* 功	能：USB特征清除函数
* 入口参数：无
* 出口参数：无
* 说	明：USB标准请求函数
*****************************************************************************************************************/
void UsbClearFeature() reentrant
{
	if ((UsbState != DEV_CONFIGURED) 
		|| (Setup.bmRequestType == IN_DEVICE) 
		|| (Setup.bmRequestType == IN_INTERFACE) 
		|| Setup.wValue.c[MSB] 
		|| Setup.wIndex.c[MSB]
		|| Setup.wLength.c[MSB] 
		|| Setup.wLength.c[LSB])
	{
		//如果参数不对，强行停止通信
		UsbForceStall();
	}

	else
	{
		if( (Setup.bmRequestType == IN_ENDPOINT) &&			
			(Setup.wValue.c[LSB] == ENDPOINT_HALT) &&		
			((Setup.wIndex.c[LSB] == IN_EP1) || 			
			(Setup.wIndex.c[LSB] == OUT_EP2))) 				
		{
			if(Setup.wIndex.c[LSB] == IN_EP1)
			{
				UsbWriteByte(INDEX, EP1_IN_IDX); 				
				UsbWriteByte(EINCSR1, rbInCLRDT);
				UsbEpStatus[1] = EP_IDLE; 						                   
			}
			else
			{
				UsbWriteByte(INDEX, EP2_OUT_IDX); 				
				UsbWriteByte(EOUTCSR1, rbOutCLRDT);
				UsbEpStatus[2] = EP_IDLE; 						
			}
		}
		else
		{
			UsbForceStall(); 								
		}
	}
	
	//正常应答
	UsbWriteByte(INDEX, EP0_IDX); 								
	if(UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}


/*****************************************************************************************************************
* 名	称：UsbSetFeature()
* 功	能：USB特征设置函数
* 入口参数：无
* 出口参数：无
* 说	明：USB标准请求函数
*****************************************************************************************************************/
void UsbSetFeature(void) reentrant
{
	if ((UsbState != DEV_CONFIGURED) ||
			(Setup.bmRequestType == IN_DEVICE) ||
			(Setup.bmRequestType == IN_INTERFACE) ||
			Setup.wValue.c[MSB] || Setup.wIndex.c[MSB]|| Setup.wLength.c[MSB]
			|| Setup.wLength.c[LSB])
	{
		UsbForceStall();
	}


	else
	{
		if ((Setup.bmRequestType == IN_ENDPOINT)&&
			(Setup.wValue.c[LSB] == ENDPOINT_HALT) &&
			((Setup.wIndex.c[LSB] == IN_EP1) || (Setup.wIndex.c[LSB] == OUT_EP2)))
		{
			if (Setup.wIndex.c[LSB] == IN_EP1)
			{
				UsbWriteByte(INDEX, EP1_IN_IDX);
				UsbWriteByte(EINCSR1, rbInSDSTL);
				UsbEpStatus[1] = EP_HALT;
			}
			else
			{
				UsbWriteByte(INDEX, EP2_OUT_IDX);
				UsbWriteByte(EOUTCSR1, rbOutSDSTL);
				UsbEpStatus[2] = EP_HALT;
			}
		}
		else
		{
			UsbForceStall();
		}
	}

	//正常应答
	UsbWriteByte(INDEX, EP0_IDX);
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}

/*****************************************************************************************************************
* 名	称：UsbSetAddress()
* 功	能：USB设备地址设置函数
* 入口参数：无
* 出口参数：无
* 说	明：USB标准请求函数
*****************************************************************************************************************/
void UsbSetAddress(void) reentrant
{
	if ((Setup.bmRequestType != IN_DEVICE) ||
			Setup.wIndex.c[MSB]  || Setup.wIndex.c[LSB] ||
			Setup.wLength.c[MSB] || Setup.wLength.c[LSB]|| 
			Setup.wValue.c[MSB]  || (Setup.wValue.c[LSB] & 0x80))
	{
		UsbForceStall();
	}

	UsbEpStatus[0] = EP_ADDRESS;
	if(Setup.wValue.c[LSB] != 0)
	{
		UsbState = DEV_ADDRESS;
	}
	else
	{
		UsbState = DEV_DEFAULT;
	}

	//正常应答
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}


/*****************************************************************************************************************
* 名	称：UsbGetDescriptor()
* 功	能：USB描述符获取函数
* 入口参数：无
* 出口参数：无
* 说	明：USB标准请求函数
*****************************************************************************************************************/
void UsbGetDescriptor(void) reentrant
{
	switch (Setup.wValue.c[MSB])
	{ 
		case DSC_DEVICE:
			UsbDataPtr = (BYTE*) &UsbDeviceDesc;
			UsbDataSize = UsbDeviceDesc.bLength;
			break;
	
		case DSC_CONFIG:
			UsbDataPtr = (BYTE*) &UsbConfigDesc;
			UsbDataSize = UsbConfigDesc.wTotalLength.c[MSB] + 256*UsbConfigDesc.wTotalLength.c[LSB];
			break;
	
		case DSC_STRING:
			UsbDataPtr = UsbStringDescTable[Setup.wValue.c[LSB]];
			UsbDataSize = *UsbDataPtr;
			break;
	
		case DSC_INTERFACE:
			UsbDataPtr = (BYTE*) &UsbInterfaceDesc;
			UsbDataSize = UsbInterfaceDesc.bLength;
			break;
	
		case DSC_ENDPOINT:
			if((Setup.wValue.c[LSB] == IN_EP1) || (Setup.wValue.c[LSB] == OUT_EP2))
			{
				if(Setup.wValue.c[LSB] == IN_EP1)
				{
					UsbDataPtr = (BYTE*) &UsbEndpoint1Desc;
					UsbDataSize = UsbEndpoint1Desc.bLength;
				}
				else
				{
					UsbDataPtr = (BYTE*) &UsbEndpoint2Desc;
					UsbDataSize = UsbEndpoint2Desc.bLength;
				}
			}
			else
			{
				UsbForceStall();
			}
			break;
	
		default:
			UsbForceStall();
			break;
	}

	if (Setup.wValue.c[MSB] == DSC_DEVICE ||
			Setup.wValue.c[MSB] == DSC_CONFIG ||
			Setup.wValue.c[MSB] == DSC_STRING || Setup.wValue.c[MSB]
			== DSC_INTERFACE || Setup.wValue.c[MSB] == DSC_ENDPOINT)
	{
		if ((Setup.wLength.c[LSB] < UsbDataSize) && (Setup.wLength.c[MSB] == 0))
		{
			UsbDataSize = Setup.wLength.i;
		}
	}

	//正常应答
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, rbSOPRDY);
		UsbEpStatus[0] = EP_TX;
		UsbDataSent = 0;
	}
}


/*****************************************************************************************************************
* 名	称：UsbGetConfiguration()
* 功	能：USB配置获取函数
* 入口参数：无
* 出口参数：无
* 说	明：USB标准请求函数
*****************************************************************************************************************/
void UsbGetConfiguration(void)  reentrant
{
	if ((Setup.bmRequestType != OUT_DEVICE) ||
			Setup.wValue.c[MSB] || Setup.wValue.c[LSB]||
			Setup.wIndex.c[MSB] || Setup.wIndex.c[LSB]||
			Setup.wLength.c[MSB] || (Setup.wLength.c[LSB] != 1))
	{
		UsbForceStall();
	}

	else
	{
		if (UsbState == DEV_CONFIGURED) 
		{
			UsbDataPtr = (BYTE*)&USB_ONES_PACKET;
			UsbDataSize = 1;
		}
		if (UsbState == DEV_ADDRESS)
		{
			UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
			UsbDataSize = 1;
		}
	}

	//正常回复
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, rbSOPRDY);
		UsbEpStatus[0] = EP_TX;
		UsbDataSent = 0;
	}
}


/*****************************************************************************************************************
* 名	称：UsbSetConfiguration()
* 功	能：USB配置设置函数
* 入口参数：无
* 出口参数：无
* 说	明：USB标准请求函数
*****************************************************************************************************************/
void UsbSetConfiguration(void)  reentrant
{

	if ((UsbState == DEV_DEFAULT) ||
			(Setup.bmRequestType != IN_DEVICE) ||
			Setup.wIndex.c[MSB] || Setup.wIndex.c[LSB]||
			Setup.wLength.c[MSB] || Setup.wLength.c[LSB] || Setup.wValue.c[MSB]
			|| (Setup.wValue.c[LSB] > 1))
	{
		UsbForceStall();
	}

	else
	{
		if (Setup.wValue.c[LSB] > 0)
		{
			UsbState = DEV_CONFIGURED;
			UsbEpStatus[1] = EP_IDLE; 
			UsbEpStatus[2] = EP_IDLE;
			UsbWriteByte(INDEX, EP1_IN_IDX);		//将索引设置到端点1上
			UsbWriteByte(EINCSR2, rbInDIRSEL); 		//将端点1设置为IN

#ifndef	USB_EP_TRANS_MASS_STORAGE					//如果不是MassStorage，下面这个操作是必须的
			UsbHandleIn();
#endif

			UsbWriteByte(INDEX, EP0_IDX); 			//将索引设置回端点0上
		}
		else
		{
			UsbState = DEV_ADDRESS;
			UsbEpStatus[1] = EP_HALT;
			UsbEpStatus[2] = EP_HALT;
		}
	}

	//正常回复
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}


/*****************************************************************************************************************
* 名	称：UsbGetInterface()
* 功	能：USB接口获取函数
* 入口参数：无
* 出口参数：无
* 说	明：USB标准请求函数
*****************************************************************************************************************/
void UsbGetInterface(void)  reentrant
{

	if ((UsbState != DEV_CONFIGURED) ||
			(Setup.bmRequestType != OUT_INTERFACE) ||
			Setup.wValue.c[MSB] ||Setup.wValue.c[LSB] ||
			Setup.wIndex.c[MSB] ||Setup.wIndex.c[LSB] ||
			Setup.wLength.c[MSB] ||(Setup.wLength.c[LSB] != 1))
	{
		UsbForceStall();
	}

	else
	{
		//回复0
		UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
		UsbDataSize = 1;
	}

	//正常回复
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, rbSOPRDY);
		UsbEpStatus[0] = EP_TX;
		UsbDataSent = 0;
	}
}


/*****************************************************************************************************************
* 名	称：UsbSetInterface()
* 功	能：USB接口设置函数
* 入口参数：无
* 出口参数：无
* 说	明：USB标准请求函数
*****************************************************************************************************************/
void UsbSetInterface(void)  reentrant
{
	if ((Setup.bmRequestType != IN_INTERFACE) ||
			Setup.wLength.c[MSB] ||Setup.wLength.c[LSB]||
			Setup.wValue.c[MSB]  ||Setup.wValue.c[LSB] || Setup.wIndex.c[MSB]
			||Setup.wIndex.c[LSB])
	{
		UsbForceStall();
	}

	//正常回复
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}


/*****************************************************************************************************************
* 名	称：OnUsbReset()
* 功	能：USB复位操作响应函数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void OnUsbReset(void) reentrant
{
	UsbState = DEV_DEFAULT; 				//将设备设置为默认状态

	UsbWriteByte(POWER, 0x01); 				//使能USB，允许挂起使能检测

	//设置每个结点的默认状态
	UsbEpStatus[0] = EP_IDLE;
	UsbEpStatus[1] = EP_HALT;
	UsbEpStatus[2] = EP_HALT;
}

/*****************************************************************************************************************
* 名	称：OnUsbSuspend()
* 功	能：USB总线挂起时的响应操作
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void OnUsbSuspend(void) reentrant
{
	//添加挂起总线时的用户处理代码
}

/*****************************************************************************************************************
* 名	称：OnUsbResume()
* 功	能：USB总线恢复时的处理函数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void OnUsbResume(void) reentrant
{
	//添加恢复总线是的用户处理代码
}


/*****************************************************************************************************************
* 名	称：OnUsbSetup()
* 功	能：USB Setup事务处理
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void OnUsbSetup(void) reentrant
{
	BYTE control_reg, TempReg; 						//保存端点信息的临时变量

	UsbWriteByte(INDEX, EP0_IDX); 					//选择端点0
	UsbReadByte(E0CSR, control_reg); 				//读取控制寄存器

	//设置地址
	if(UsbEpStatus[0] == EP_ADDRESS)
	{
		UsbWriteByte(FADDR, Setup.wValue.c[LSB]);
		UsbEpStatus[0] = EP_IDLE;
	}

	//清除异常标志
	if (control_reg & rbSTSTL)
	{
		UsbWriteByte(E0CSR, 0);
		UsbEpStatus[0] = EP_IDLE;
		return;
	}

	//事务处理完成
	if (control_reg & rbSUEND)
	{
		UsbWriteByte(E0CSR, rbDATAEND);
		UsbWriteByte(E0CSR, rbSSUEND);
		UsbEpStatus[0] = EP_IDLE;
	}

	//如果处于空闲模式
	if(UsbEpStatus[0] == EP_IDLE)
	{
		//判断端点0是否有数据可读取，如果有，则处理
		if (control_reg & rbOPRDY)
		{
			UsbFifoRead(FIFO_EP0, (BYTE *)&Setup, 8);	   //读取Setup事物处理包

			//转换大于一个字节的数据类型的编码，应为PC上和KEIL编译出来的不一样
			Setup.wValue.i = Setup.wValue .c[MSB] + 256*Setup.wValue.c[LSB];
			Setup.wIndex.i = Setup.wIndex .c[MSB] + 256*Setup.wIndex.c[LSB];
			Setup.wLength.i = Setup.wLength.c[MSB] + 256*Setup.wLength.c[LSB];

			//处理USB的标准请求命令
			switch (Setup.bRequest)
			{
				case GET_STATUS:
					UsbGetStatus();
					break;
	
				case CLEAR_FEATURE:
					UsbClearFeature();
					break;
	
				case SET_FEATURE:
					UsbSetFeature();
					break;
	
				case SET_ADDRESS:
					UsbSetAddress();
					break;
	
				case GET_DESCRIPTOR:
					UsbGetDescriptor();
					break;
	
				case GET_CONFIGURATION:
					UsbGetConfiguration();
					break;
	
				case SET_CONFIGURATION:
					UsbSetConfiguration();
					break;
	
				case GET_INTERFACE:
					UsbGetInterface();
					break;
	
				case SET_INTERFACE:
					UsbSetInterface();
					break;
					 
				default:
					UsbForceStall();		//如果命令无效，则强制停止
					break;
			}
		}
	}

	//查看端点0是否有数据需要发送
	if(UsbEpStatus[0] == EP_TX)
	{
		//确保发送FIFO队列是空的
		if((control_reg & rbINPRDY) == 0)
		{
			//读取当前的E0CSR寄存器
			UsbReadByte(E0CSR, control_reg);

			//确保通信没有被终止并且没有接收到新的数据
			if((!(control_reg & rbSUEND)) || (!(control_reg & rbOPRDY)))
			{
				TempReg = rbINPRDY;              

				//如果数据长度比端点数据容量大，将数据分段
				if (UsbDataSize >= EP0_PACKET_SIZE)
				{
					UsbFifoWrite(FIFO_EP0, (BYTE*)UsbDataPtr, EP0_PACKET_SIZE);
					UsbDataPtr += EP0_PACKET_SIZE;
					UsbDataSize -= EP0_PACKET_SIZE;
					UsbDataSent += EP0_PACKET_SIZE;
				}
				else
				{
					UsbFifoWrite(FIFO_EP0, (BYTE *)UsbDataPtr, UsbDataSize);
					TempReg |= rbDATAEND; 			//设置数据结束标志
					UsbEpStatus[0] = EP_IDLE; 		//将端点0状态设置为空闲状态
				}

				//特殊情况
				if (UsbDataSent == Setup.wLength.i)
				{
					TempReg |= rbDATAEND; 			//设置数据结束标志
					UsbEpStatus[0] = EP_IDLE; 		//将端点0状态设置为空闲状态
				}

				//向控制寄存器写入指定操作
				UsbWriteByte(E0CSR, TempReg); // Write mask to E0CSR
			}
		}
	}
}

/*****************************************************************************************************************
* 名	称：UsbIsr()
* 功	能：USB中断处理函数
* 入口参数：无
* 出口参数：无
* 说	明：处理USB中断信息
*****************************************************************************************************************/
void UsbIsr(void) interrupt 8
{
	BYTE bCommon, bIn, bOut;

	//进入中断
	OSIntEnter();

	//读取中断所有USB寄存器=====================================================================
	UsbReadByte(CMINT, bCommon); 				
	UsbReadByte(IN1INT, bIn); 					
	UsbReadByte(OUT1INT, bOut);

	//处理======================================================================================
	
	//恢复总线时的处理
	if (bCommon & rbRSUINT)
	{
		OnUsbResume();
	}

 	//复位处理
	if (bCommon & rbRSTINT)
	{
		OnUsbReset();
	}

	//Setup事物处理
	if (bIn & rbEP0)
	{
		OnUsbSetup();
	}

	//向主机发送数据的处理操作
	if (bIn & rbIN1)
	{
		UsbHandleIn();
	}

	//从主机接收数据的处理操作
	if (bOut & rbOUT2)
	{
		UsbHandleOut();
	}

	//挂起总线时的处理
	if (bCommon & rbSUSINT)
	{
		OnUsbSuspend();
	}

	//退出中断
	OSIntExit();
}
