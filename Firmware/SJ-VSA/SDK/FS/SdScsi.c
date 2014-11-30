/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: SdScsi.c
**创   建   人: 杨承凯(kady1984@163.com)
**创 建 日  期: 2008年04月20日
**最后修改日期: 2008年04月20日
**描        述: SCSI协议实现函数
*****************************************************************************************************************/
#include <string.h>
#include "SdSet.h"
#include "SdSectServer.h"
#include "SdScsi.h"
#include "..\Communication\UsbInclude.h"

//=========================================bulk-only传输简介=============================================
//
//bulk-only传输就是主机先对U盘的Out端点发送Out令牌封包，将包含31个字节的cbw通过数据封包发送至Out端点，如果
//有命令需要传送至主机的话则向In端点发送In令牌后再接收所需要的数据，完成后再向In端点发送In令牌，接收CSW
//
//=======================================================================================================



//CBW是主机通过Bulk-Out端点向设备发送的命令块包，在CBW中使用方向位和数据传输长度域指明期
//待的传输，CBW必须起始于包边界，并且必须以31字节的短包传输结束，相继的数据包和CSW包必须
//开始于一个新的包边界，所有的CBW包必须按低字节在前的次序传输
typedef struct
{
	DWORD dCBWSignature; 				//命令块包标识：CBW包标记，表明这是一个CBW包，这个域的值为0x43425355
	DWORD dCBWTag;						//命令块标记：当设备返回相应的CSW包时，必须使命令状态标记域的值与此值相同
	DWORD dCBWDataTransferLength;		//数据传输长度：指明命令执行期间在Bulk端点上传数据的字节长度
	BYTE bmCBWFlags;					//命令块标旗：方向位规定了Bulk端点数据传输的方向，其他位预留
	BYTE bCBWLUN;						//逻辑单元号：指定命令块被发送到的逻辑单元号，如果设备不支持多个逻辑单元号，则主机将这个域设置为0
	BYTE bCBWCBLength;					//CBWCB长度：定义了CBWCB的有效长度，合法值为1-16
	BYTE CBWCB[16];						//CBWCB。由设备执行的命令，由设备解释
} CBW;


//CSW向主机表明来自于CBW包的命令块的执行状态。设备收到CBW包解析处理后将通过Bulk-In端点发送一个CSW包
typedef struct
{
	DWORD dCSWSignature; 				//命令状态包标识：CSW包的标记，表明这是一个CSW包，这个域的值为53425355H
	DWORD dCSWTag;						//命令状态标记：次域的值域CBW包的命令块标记相同
	DWORD dCSWDataResidue;				//数据残余：实际数据传输量与CBW包中规定的数据传输长度的差值
	BYTE bCSWStatus;					//命令执行状态：表明命令成功或失败信息(0 - 成功，非0 - 失败)
} CSW;




//============================================Mass Storage 类简介=============================================
//
//USB设备分为5大类，即显示器、通信设备、音频设备、人机输入和海量存储。通常所用的U盘、移动硬盘均属于海量存储类。
//海量存储类的规范中包括4个独立的子规范，即CBI传输、Bulk-Only传输、ATA命令块、UFI命令规范。前两个协议定义了数
//据\命令\状态在USB总线上的传输方法，Bulk-Only传输协议仅仅使用Bulk端点传送数据/命令/状态，CBI传输协议则使用
//Control/bulk/interrupt三种类型的端点进行数据/命令/状态的传送。后两个协议定义了存储介质的操作命令，ATA协议用
//于硬盘，UFI协议则针对USB移动存储，U盘读写器的设计遵循Bulk-Only传输协议和UFI命令规范。UFI命令块规范是针对USB
//移动存储而制定的，它总共定义了19个12字节长度的操作命令。
//
//============================================================================================================




//=================================================UFI命令码定义==============================================
//
//UFI是SCSI的子集
//
//============================================================================================================
#define SCSI_TEST_UNIT_READY 				0x00					//请求设备报告是否处于Ready状态
#define SCSI_REQUEST_SENSE 					0x03					//请求设备向主机返回执行结果，及状态数据
#define SCSI_FORMAT_UNIT					0x04	  				//格式化存储单元
#define SCSI_SEND_DIAGNOSTIC				0x10
#define SCSI_INQUIRY						0x12					//索取器件信息
#define SCSI_MODE_SELECT_6					0x15
#define SCSI_MODE_SENSE_6					0x1A
#define SCSI_START_STOP_UNIT				0x1B					//load/unload
#define SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL 	0x1E					//写保护
#define SCSI_READ_CAPACITY_10				0x25					//要求设备返回当前容量
#define SCSI_READ_CAPACITY_16				0x9E
#define SCSI_READ_6							0x08
#define SCSI_READ_10						0x28					//Host读存储介质中的二进制数据
#define SCSI_READ_16						0x88
#define SCSI_WRITE_10						0x2A					//从主机向介质写二进制数据
#define SCSI_VERIFY_10						0x2F					//在存储中验证数据
#define SCSI_READ_FORMAT_CAPACITIES 		0x23					//查询当前容量及可用空间

//SCSI标准参数定义=============================================================================================
#define DIRECTION_IN	0x80
#define DIRECTION_OUT	0x00
#define CBW_SIGNATURE 				0x55534243
#define CSW_SIGNATURE 				0x55534253

//SCSI协议流程步骤定义=========================================================================================
#define MSD_READY 					0x00
#define MSD_COMMAND_TRANSPORT 		0x01
#define MSD_DATA_IN					0x02
#define MSD_DATA_OUT				0x03
#define MSD_STATUS_TRANSPORT		0x04
#define MSD_DATA					0x05
#define MSD_DO_RESET				0xFF

//SCSI状态定义
#define SCSI_PASSED 		0
#define SCSI_FAILED 		1
#define SCSI_PHASE_ERROR 	2

//SCSI信息字段定义=================================================================================================
//模式信息
code const BYTE Scsi_Mode_Sense_6[4]= {0x03, 0, 0, 0}; 					// No mode sense parameter
//标准的设备信息
code const BYTE Scsi_Standard_Inquiry_Data[28]=
{
	0x00, 																// Peripheral qualifier & peripheral device type
	0x80, 																// Removable medium
	0x05, 																// Version of the standard (2=obsolete, 5=SPC-3)
	0x02, 																// No NormACA, No HiSup, response data format=2
	0x1F, 																// No extra parameters
	0x00, 																// No flags
	0x80, 																// 0x80 => BQue => Basic Task Management supported
	0x00, 																// No flags
	'K','a','d','y',' ',' ',' ',' ',
	'M','a','s','s',' ',
	'S','t','o','r','a','g','e' 										// Requested by Dekimo via www.t10.org
};
//磁盘信息
BYTE xdata Scsi_Read_Capacity_10[8]=
{
	0x00,0x00,0xF4,0x5F, 											// Last logical block address
	0x00,0x00,msb(SectBlockSize()),lsb(SectBlockSize()) 			// Block length
};

//内部全局变量定义================================================================================================
BYTE xdata MsdState = MSD_READY;
BYTE xdata ScsiStatus;
DWORD xdata ScsiResidue;
CBW xdata cbw;
CSW xdata csw;


//函数定义========================================================================================================

/*****************************************************************************************************************
* 名	称：ScsiSend()
* 功	能：发送SISC数据
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static void ScsiSend(BYTE* ptr, unsigned count) reentrant
{
	if (ScsiResidue < count)
	{
		ScsiStatus = SCSI_PHASE_ERROR;
		return;
	}
	ScsiResidue -= count;
	Usb0DeviceSend(ptr, count, 0);
}

/*****************************************************************************************************************
* 名	称：ScsiInquiry()
* 功	能：查询USB设备信息
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void ScsiInquiry() reentrant
{
	ScsiStatus=SCSI_PASSED;
	ScsiSend(Scsi_Standard_Inquiry_Data, sizeof(Scsi_Standard_Inquiry_Data));
}

/*****************************************************************************************************************
* 名	称：ScsiReadCapacity10()
* 功	能：查询SD卡的容量
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void ScsiReadCapacity10() reentrant
{
	unsigned int s;
	unsigned long size = SdSectors();
	size -= 1;
	s = ((size&0xFFFF0000) >> 16);
	Scsi_Read_Capacity_10[0]=msb((s));
	Scsi_Read_Capacity_10[1]=lsb((s));
	Scsi_Read_Capacity_10[2]=msb(size);
	Scsi_Read_Capacity_10[3]=lsb(size);

	ScsiStatus = SCSI_PASSED;
	ScsiSend(Scsi_Read_Capacity_10, sizeof(Scsi_Read_Capacity_10));
}

/*****************************************************************************************************************
* 名	称：ScsiRead10()
* 功	能：SCSI读数据
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void ScsiRead10() reentrant
{
	int i, j;
	DWORD xdata d_len = ntohl(cbw.dCBWDataTransferLength);
	DWORD xdata d_LBA = cbw.CBWCB[2];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[3];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[4];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[5];

	for (i=0; i<(d_len+SectBlockSize()-1)/SectBlockSize(); i++)
	{
		SectRead(d_LBA+i);
		for (j = 0; j < (SectBlockSize()+EP1_PACKET_SIZE-1)/EP1_PACKET_SIZE; j++)
		{
			Usb0DeviceSend(Scratch + j * EP1_PACKET_SIZE, EP1_PACKET_SIZE, 0);
			ScsiResidue -= EP1_PACKET_SIZE;
		}
	}

	ScsiStatus = SCSI_PASSED;
}

/*****************************************************************************************************************
* 名	称：ScsiWrite10()
* 功	能：写数据到SD卡
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void ScsiWrite10() reentrant
{
	int i, j;
	DWORD xdata d_len = ntohl(cbw.dCBWDataTransferLength);
	DWORD xdata d_LBA = cbw.CBWCB[2];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[3];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[4];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[5];

	for (i=0; i<(d_len+SectBlockSize()-1)/SectBlockSize(); i++)
	{
		for (j=0; j<(SectBlockSize()+EP2_PACKET_SIZE-1)/EP2_PACKET_SIZE; j++)
		{
			//kady begin
			Usb0DeviceRecv(Scratch+j*EP2_PACKET_SIZE, EP2_PACKET_SIZE, 0);
			//kady end
		}
		SectWrite(d_LBA+i);
		ScsiResidue-=SectBlockSize();
	}
	ScsiStatus=SCSI_PASSED;
}

/*****************************************************************************************************************
* 名	称：ScsiModeSense6()
* 功	能：SCSI模式查询
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void ScsiModeSense6()	reentrant
{
	ScsiStatus = SCSI_PASSED;
	ScsiSend(Scsi_Mode_Sense_6, sizeof(Scsi_Mode_Sense_6));
}

/*****************************************************************************************************************
* 名	称：ScsiRx()
* 功	能：SCSI数据接收
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void ScsiRx() reentrant
{
	int xdata i;

	ScsiStatus = SCSI_FAILED;
	ScsiResidue = ntohl(cbw.dCBWDataTransferLength);

	if (cbw.bCBWCBLength == 0) return; 					//检查数据长度有效性

	//解析SCSI命令
	switch (cbw.CBWCB[0])
	{
		case SCSI_TEST_UNIT_READY:
			ScsiStatus = SCSI_PASSED;
			break;

		case SCSI_INQUIRY:
			ScsiInquiry();
			break;

		case SCSI_MODE_SENSE_6:
			ScsiModeSense6();
			break;

		case SCSI_READ_CAPACITY_10:
			ScsiReadCapacity10();
			break;

		case SCSI_READ_10:
			ScsiRead10();
			break;

		case SCSI_WRITE_10:
			ScsiWrite10();
			break;

		case SCSI_VERIFY_10:
			ScsiResidue = 0;
			ScsiStatus = SCSI_PASSED;
			break;

		case SCSI_START_STOP_UNIT:
			ScsiStatus = SCSI_PASSED;
			break;

		case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
			ScsiStatus = SCSI_PASSED;
			break;

		default:
			break;
	}

	if (ScsiResidue && (ScsiResidue == ntohl(cbw.dCBWDataTransferLength)))
	{
		//清零发送缓冲区
		for (i=0; i<EP1_PACKET_SIZE; i++)
		{
			UsbInBuffer[i]=0;
		}

		//向主机发送空余数据
		while(ScsiResidue)
		{
			Usb0DeviceSend(UsbInBuffer, (EP1_PACKET_SIZE>ScsiResidue) ? ScsiResidue : EP1_PACKET_SIZE, 0);
			ScsiResidue -= ((EP1_PACKET_SIZE>ScsiResidue) ? ScsiResidue : EP1_PACKET_SIZE);
		}
	}
}

/*****************************************************************************************************************
* 名	称：SdScsiServerStep()
* 功	能：SCSI协议操作流程
* 入口参数：无
* 出口参数：无
* 说	明：在用户程序中应当循环调用此操作
*****************************************************************************************************************/
void SdScsiServerStep() reentrant
{
	unsigned nRecvCount;

	switch (MsdState)
	{
		case MSD_READY:
			nRecvCount = Usb0DeviceRecv(UsbOutBuffer, 1000, 0);

			//检查是不是CBW包
			if (nRecvCount != sizeof(CBW))
			{
				return;
			}
	
			//如果是
			memcpy(&cbw, UsbOutBuffer, nRecvCount);
	
			//检查标志位的有效性
			if ((cbw.dCBWSignature!=CBW_SIGNATURE) 
					|| ((cbw.bmCBWFlags != DIRECTION_IN && cbw.bmCBWFlags != DIRECTION_OUT)
					|| (cbw.bCBWLUN&0xF0) 
					|| (cbw.bCBWCBLength>16))
					|| (cbw.bCBWLUN!=0x00))
			{
				return;
			}
			
			//切换到数据读取状态	
			MsdState = MSD_DATA;
			break;
	
		case MSD_DATA:
			ScsiRx();										//按SCSI协议读取数据
			MsdState = MSD_STATUS_TRANSPORT;  				//切换到数据发送状态
			break;
	
		case MSD_STATUS_TRANSPORT:
			//回复一个CSW结构:
			csw.dCSWSignature = CSW_SIGNATURE;
			csw.dCSWTag = cbw.dCBWTag;
			csw.bCSWStatus = ScsiStatus;
			csw.dCSWDataResidue = ntohl(ScsiResidue);
	
			//发送数据
			Usb0DeviceSend((BYTE*)&csw, sizeof(CSW), 0);
	
			MsdState = MSD_READY;							//切换到就绪状态
			break;
	
		case MSD_DO_RESET:
			break;
	
		default:
			MsdState = MSD_READY;							//默认到就绪状态
			break;
	}
}



