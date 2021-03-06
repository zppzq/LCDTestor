/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: UsbDescriptor.h
**创   建   人: 杨承凯
**创 建 日  期: 2008年04月10日
**最后修改日期: 2008年04月10日
**描        述: USB描述符结构定义文件
*****************************************************************************************************************/
#ifndef  _USB_DESC_H_
#define  _USB_DESC_H_

//基本数据类型定义======================================================================================
#ifndef _BYTE_DEF_
#define _BYTE_DEF_
typedef unsigned char BYTE;
#endif

//基本类型结构定义
#ifndef _WORD_DEF_
#define _WORD_DEF_
typedef union
{
	unsigned int i;
	unsigned char c[2];
} WORD;
#define LSB 1
#define MSB 0
#endif

//==========================================描述符说明==================================================
//
//USB是通过USB描述符来对USB设备进行属性的说明，包括使用的协议、接口数目、端点和传输方式等等。当USB设备插
//入主机后，主机要对其进行总线枚举，配置该设备所需的驱动等信息。主机通过标准请求GetDescriptor来读取USB的
//描述符，从而得到设备的相关信息，，然后建立通信。因此说，只有正确设置USB的描述符，才能使USB设备正常的工
//作起来。
//
//标准的USB设备有5种USB描述符，分别为设备描述符、配置描述符、接口描述符、端点描述符和字符串描述符。每一个
//USB设备都有一个设备描述符。而每一个设备描述符有一个默认的配置描述符，配置描述符主要定义了USB设备的功能，
//如果USB支持多个功能，那样就需要为每个功能定义一个配置。但是同一时刻只有一个配置可用。一个配置支持至少一
//个接口，接口定义了实现功能的硬件的集合。每一个能够与USB实现数据交换的硬件叫做端点，也就是说接口是端点的
//集合。字符串描述符是一个连续的数字。
//
//描述符之间有一定的关系，一个设备只有一个设备描述符，而一个设备描述符可以包含多个配置描述符，而一个配置
//描述符可以包含多个接口描述符，一个接口使用了几个端点，就有几个端点描述符
//
//======================================================================================================



//===============================设备描述符(Device Descriptor)==========================================
//
//设备描述符用于指出USB设备的总体信息
//
//======================================================================================================
typedef code struct
{
	BYTE bLength; 					//描述符的长度(字节数)
	BYTE bDescriptorType; 			//描述符类型，为0x01
	WORD bcdUSB; 					//USB的版本号(用BCD码表示)
	BYTE bDeviceClass; 				//设备的类型代码，0x00表示在接口描述符里定义类型，0x01-0xFE为标准设备类，0xff为厂商自定义类型
	BYTE bDeviceSubClass; 			//设备子类型代码，同上(上一项为0时，此项必须为0)
	BYTE bDeviceProtocol; 			//设备的协议代码，同上(上一项为0时，此项必须为0)
	BYTE bMaxPacketSize0; 			//端点0的最大包的大小 
	WORD idVendor; 					//厂商编号(需要向USB组织申请) 
	WORD idProduct; 				//产品编号
	WORD bcdDevice; 				//设备出厂编号(BCD码)
	BYTE iManufacturer; 			//描述厂商字符串的索引
	BYTE iProduct; 					//描述产品字符串的索引
	BYTE iSerialNumber; 			//描述设备序列号字符串的索引
	BYTE bNumConfigurations; 		//配置描述符的数量
} CUsbDeviceDesc;

 
//=============================配置描述符(Configuration Descriptor)=====================================
//
//USB设备的一个配置两包含一个或多个接口，且每个接接口都可以相互独立地工作。主机使用的SetConfiguration请求
//为USB设备选择一个合适的配置，使用GetConfiguration请求读取USB设备当前的配置。USB设备同一时间只能使用一个
//配置，所有的USB设备要至少支持一个配置描述符。
//
//======================================================================================================
typedef code struct
{
	BYTE bLength; 					//描述符的长度(字节数)
	BYTE bDescriptorType; 			//描述符类型，为0x02
	WORD wTotalLength; 				//配置所返回的所有数量的大小
	BYTE bNumInterfaces; 			//此配置所支持的接口数量
	BYTE bConfigurationValue; 		//此配置的编号
	BYTE iConfiguration; 			//描述该配置的字符串的索引值(0表示没有)
	BYTE bmAttributes; 				//供电模式的选择
	BYTE bMaxPower; 				//设备从总线提取的最大电流(单位为2mA)
} CUsbConfigDesc;


//===================================接口描述符(Interface Descriptor)===================================
//
//USB设备的接口是一个端点的集合，它负责完成该设备的特定功能，如光盘驱动器就可以具有大容量存储接口和音频接
//口。USB设备同一配置的各个接接口间不能使用相同的端点。
//
//======================================================================================================
typedef code struct
{
	BYTE bLength; 					//描述符的长度(字节数)
	BYTE bDescriptorType; 			//描述符类型，为0x04
	BYTE bInterfaceNumber; 			//接口的编号
	BYTE bAlternateSetting; 		//备用的接口描述符编号
	BYTE bNumEndpoints; 			//该接口使用端点数，不包括端点0
	BYTE bInterfaceClass; 			//接口类型
	BYTE bInterfaceSubClass; 		//接口子类型
	BYTE bInterfaceProtocol; 		//接口所遵循的协议
	BYTE iInterface; 				//描述该接口的字符串索引值(0表示没有)
} CUsbInterfaceDesc;


//===========================================端点描述符=================================================
//
//端点描述符用于指出USB设备端点的特性，如其所支持的传输类型、传输方向等信息。除端点0外，USB设备的每个端点
//都必须有一个端点描述符。
//
//======================================================================================================
typedef code struct
{
	BYTE bLength; 					//描述符的长度(字节数)
	BYTE bDescriptorType; 			//描述符类型，为0x05
	BYTE bEndpointAddress; 			//端点地址及输入输出属性(Bit8为1代表输入端点，为0代表输出端点；后边的位代表端点号)
	BYTE bmAttributes; 				//端点的传输类型属性(0x00 - 控制，0x01 - 同步，0x02 - 批量，0x03 - 中断)
	WORD wMaxPacketSize; 			//端点收、发的最大包的大小(字节数)
	BYTE bInterval; 				//主机查询端点的时间间隔(毫秒)，只在中断传输方式和同步传输方式时有用（同步传输时必须取1)
} CUsbEndpointDesc;


//==================================字符串描述符(String Descriptor)=====================================
//
//字符串描述符用于保存一些文本信息，如供应商名称、产品序列号等。它是可选的。在其他描述符中，可以含有指向
//字符串描述符的索引值。如果其值为1，则表示其引用的是第一个字符串描述符；如果其值为0，则表示该字段没有引
//用任何字符申描述符。
//
//======================================================================================================
typedef code struct
{
	BYTE bLength; 					//描述符的长度(字节数)
	BYTE bDescriptorType; 			//描述符类型，为0x03
	BYTE SomeDescriptor[36];		//UNICODE编码的字符串
} CUsbStringDesc;

//========================================Setup事务结构=================================================
//
//USB在Setup事务处理的时候，主机总是向从机发送8个字节的数据包，定义的命令的方向、类型、和接收者以及命令代
//码，同时带了两个与命令相关的字段变量。
//
//======================================================================================================
typedef struct
{
	BYTE bmRequestType; 			//请求命令的方向、类型、和接收者
	BYTE bRequest; 					//请求命令的代码
	WORD wValue; 					//与命令代码相关的参数
	WORD wIndex; 					//与命令代码相关的参数
	WORD wLength;					//需要传输的字节数
} CUsbSetupCmd;
//bmRequestType字段意义 [B7]：			0 - 主机到设备；1 - 设备到主机
//						[B6B5]：		00 - 标准请求命令；01 - 类请求命令；10 - 用户定义命令；11 - 保留未使用
//						[B4B3B2B1B0]：	00000 - 接收者为设备；
//										00001 - 接收者为接口；
//										00010 - 接收者为端点；
//										00011 - 接收者为其他接收者；
//										其他 - 保留


//数据定义========================================================================================================
extern CUsbDeviceDesc 		UsbDeviceDesc; 			//设备描述符
extern CUsbConfigDesc 		UsbConfigDesc;			//配置描述符
extern CUsbInterfaceDesc 	UsbInterfaceDesc;		//接口描述符
extern CUsbEndpointDesc 	UsbEndpoint1Desc;		//端点1描述符
extern CUsbEndpointDesc 	UsbEndpoint2Desc;		//端点2描述符
extern BYTE* UsbStringDescTable[];					//字符串描述符

#endif  //_USB_DESC_H_
