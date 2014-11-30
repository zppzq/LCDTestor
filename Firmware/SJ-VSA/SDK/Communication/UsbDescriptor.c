/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: UsbDescriptor.c
**创   建   人: 杨承凯
**创 建 日  期: 2008年04月12日
**最后修改日期: 2008年04月12日
**描        述: USB描述符设置文件
*****************************************************************************************************************/
#include "UsbInclude.h"
#include "UsbDescriptor.h"


//参数宏定义============================================================================================

//描述符类型定义
#define DEVICE_DESCRIPTOR               0x01  //设备描述符
#define CONFIGURATION_DESCRIPTOR        0x02  //配置描述符
#define STRING_DESCRIPTOR               0x03  //字符串描述符
#define INTERFACE_DESCRIPTOR            0x04  //接口描述符
#define ENDPOINT_DESCRIPTOR             0x05  //端点描述符

//端点传输类型定义
#define ENDPOINT_TYPE_CONTROL           0x00 //控制传输
#define ENDPOINT_TYPE_ISOCHRONOUS       0x01 //同步传输
#define ENDPOINT_TYPE_BULK              0x02 //批量传输
#define ENDPOINT_TYPE_INTERRUPT         0x03 //中断传输



//定义并初始化描述符====================================================================================

//此设备的描述符
const CUsbDeviceDesc UsbDeviceDesc =
{ 
	0x12, 								//描述符长度
	0x01, 								//描述符类型
	0x1001, 							//USB设备版本号(用BCD码表示)
	0x00, 								//USB设备类代码，0x01-0xFE为标准设备类，0xff为厂商自定义类型
	0x00, 								//子类代码，同上，值由USB规定和分配的
	0x00, 								//设备协议代码，同上
	EP0_PACKET_SIZE, 					//端点0的最大包的大小
	0xC410, 							//厂商编号
	USB_PRODUCT_CLASS_TYPE,				//产品编号，跟厂商编号一起配合使用，让主机注册该设备并加载相应的驱动程序 
	0x0000, 							//设备出厂编号(BCD码)
	0x01, 								//描述厂商字符串的索引
	0x00, 								//描述产品字符串的索引   
	0x03, 								//描述设备序列号字符串的索引
	0x01 								//配置描述符的数量
};

//配置描述符
const CUsbConfigDesc UsbConfigDesc =
{
	0x09, 								//描述符长度
	0x02, 								//描述符类型
	0x2000, 							//配置所返回的所有数量的大小
	0x01, 								//此配置所支持的接口数量
	0x01, 								//该配置的编号
	0x00, 								//描述该配置的字符串的索引值
	0x80, 								//总线供电，不支持远程唤醒
	0x0F 								//设备从总线提取的最大电流(此处为30mA)
};

//接口描述符
const CUsbInterfaceDesc UsbInterfaceDesc =
{
	0x09, 								//描述符长度
	0x04, 								//描述符类型
	0x00, 								//接口的编号
	0x00, 								//备用的接口编号
	0x02, 								//该接口使用端点数，不包括端点0
	USB_INTERFACE_TRANS_TYPE, 			//接口类型(0x08代表MASS STORAGE DEVICE) 
	USB_INTERFACE_TRANS_SUB_TYPE, 		//接口子类型(0x06代表SCSI Transparent command set，即简化块命令)
	USB_INTERFACE_TRANS_PROTACAL, 		//接口所遵循的协议(0x50代表 BULK-ONLY 协议，即单批量传输协议)
	0x00 								//描述该接口的字符串索引值
};

//端点1描述符
const CUsbEndpointDesc UsbEndpoint1Desc =
{
	0x07, 								//描述符长度
	0x05,			 					//描述符类型
	0x81, 								//端点地址及输入输出属性(IN)
	USB_EP_TRANS_TYPE, 					//端点的传输类型属性
	SWAP_WORD(EP1_PACKET_SIZE), 		//端点收、发的最大包的大小(LITTLE ENDIAN)
	USB_EP_SCAN_TIME 					//主机查询端点的时间间隔(毫秒)
}; 

//端点2描述符
const CUsbEndpointDesc UsbEndpoint2Desc =
{
	0x07, 								//描述符长度
	0x05, 								//描述符类型
	0x02, 								//端点地址及输入输出属性(OUT)
	USB_EP_TRANS_TYPE, 					//端点的传输类型属性
	SWAP_WORD(EP2_PACKET_SIZE), 		//端点收、发的最大包的大小(LITTLE ENDIAN)
	USB_EP_SCAN_TIME 					//主机查询端点的时间间隔(毫秒)
}; 


//描述产品的字符串
#define STR0LEN 4
code const BYTE UsbProductStringDesc[STR0LEN] =
{
	STR0LEN, 0x03, 
	0x09, 0x04
};

//描述厂商的字符串
#define STR1LEN sizeof("KadyUsbDevice")*2
code const BYTE UsbCompanyStringDesc[STR1LEN] =
{
	STR1LEN, 0x03,
	'K', 0,
	'a', 0,
	'd', 0,
	'y', 0,
	'U', 0,
	's', 0,
	'b', 0,
	'D', 0,
	'e', 0,
	'v', 0,
	'i', 0,
	'c', 0,
	'e', 0,
};

//设备描述字符串
#define STR2LEN sizeof("UsbCommDevice")*2
code const BYTE UsbDeviceStringDesc[STR2LEN] =
{
	STR2LEN, 0x03,
	'U', 0,
	's', 0,
	'b', 0,
	'C', 0,
	'o', 0,
	'm', 0,
	'm', 0,
	'D', 0,
	'e', 0,
	'v', 0,
	'i', 0,
	'c', 0,
	'e', 0,
};

//描述设备序列号字符串的索引
#define STR3LEN sizeof("0079876543210")*2
code const BYTE UsbSerialStringDesc[STR3LEN] =
{
	STR3LEN, 0x03,
	'0', 0,
	'0', 0,
	'7', 0,
	'9', 0,
	'8', 0,
	'7', 0,
	'6', 0,
	'5', 0,
	'4', 0,
	'3', 0,
	'2', 0,
	'1', 0,
	'0', 0
};

//排列字符串
BYTE* const UsbStringDescTable[] = {UsbProductStringDesc, UsbCompanyStringDesc, UsbDeviceStringDesc, UsbSerialStringDesc};





