/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: UsbInclude.h
**创   建   人: 杨承凯
**创 建 日  期: 2008年04月17日
**最后修改日期: 2008年04月17日
**描        述: USB标准量定义
*****************************************************************************************************************/
#ifndef _USB_STAND_DEFINE_H_
#define _USB_STAND_DEFINE_H_


//描述符类型定义(由USB标准所规定)===================================================================================
#define  DSC_DEVICE              0x01        //设备描述符
#define  DSC_CONFIG              0x02        //配置描述符
#define  DSC_STRING              0x03        //字符描述符
#define  DSC_INTERFACE           0x04        //接口描述符
#define  DSC_ENDPOINT            0x05        //端点描述符

//HID描述符定义=====================================================================================================
#define DSC_HID					0x21		// HID Class Descriptor
#define DSC_HID_REPORT			0x22		// HID Report Descriptor

// HID Request Codes================================================================================================
#define GET_REPORT 				0x01		// Code for Get Report
#define GET_IDLE				0x02		// Code for Get Idle
#define GET_PROTOCOL			0x03		// Code for Get Protocol
#define SET_REPORT				0x09		// Code for Set Report
#define SET_IDLE				0x0A		// Code for Set Idle
#define SET_PROTOCOL			0x0B		// Code for Set Protocol

//定义设备的状态====================================================================================================
#define  DEV_ATTACHED           0x00       	//接入态：设备接入主机后，主机通过检测信号线上的电平变化来发现设备的接入
#define  DEV_POWERED            0x01   		//供电态：就是给设备供电，分为设备接入时的默认供电值，配置阶段后的供电值
#define  DEV_DEFAULT            0x02      	//缺省态：USB在被配置之前，通过缺省地址0与主机进行通信
#define  DEV_ADDRESS            0x03     	//地址态：经过了配置，USB设备被复位后，就可以按主机分配给它的唯一地址来与主机通信，这种状态就是地址态
#define  DEV_CONFIGURED         0x04       	//配置态：通过各种标准的USB请求命令来获取设备的各种信息，并对设备的某此信息进行改变或设置
#define  DEV_SUSPENDED          0x05       	//挂起态：总线供电设备在3ms内没有总线动作，即USB总线处于空闲状态的话，该设备就要自动进入挂起状态

//定义端点状态======================================================================================================
#define  EP_IDLE                0x00        //空闲状态
#define  EP_TX                  0x01        //发送状态
#define  EP_RX                  0x02        //接收状态
#define  EP_HALT                0x03        //停止状态
#define  EP_STALL               0x04        //即将停止
#define  EP_ADDRESS             0x05        //即将改变地址

//Setup事务中请求命令的代码(bRequest字段)===========================================================================
#define  GET_STATUS             0x00        //返回特定接收者的状态
#define  CLEAR_FEATURE          0x01        //清除或禁止接收者的某些特性
#define  SET_FEATURE            0x03        //启用或激活命令接收者的某些特性
#define  SET_ADDRESS            0x05        //给设备分配地址
#define  GET_DESCRIPTOR         0x06        //用于主机获取设备的特定描述符
#define  SET_DESCRIPTOR         0x07        //修改设备中有关的描述符，或者增加新的描述符
#define  GET_CONFIGURATION      0x08        //主机获取设备当前设备的配置值
#define  SET_CONFIGURATION      0x09        //主机指示设备采用的要求的配置
#define  GET_INTERFACE          0x0A        //获取当前某个接口描述符编号
#define  SET_INTERFACE          0x0B        //主机要求设备用某个描述符来描述接口
#define  SYNCH_FRAME            0x0C        //用于设备设置和报告一个端点的同步帧

//Setup事务中请求命令的接收者和方向的定义(bmRequestType字段)========================================================
#define  IN_DEVICE               0x00        //接收者为设备, 方向是IN 
#define  OUT_DEVICE              0x80        //接收者为设备, 方向是OUT
#define  IN_INTERFACE            0x01        //接收者为接口, 方向是IN
#define  OUT_INTERFACE           0x81        //接收者为接口, 方向是OUT
#define  IN_ENDPOINT             0x02        //接收者为端点, 方向是IN
#define  OUT_ENDPOINT            0x82        //接收者为端点, 方向是OUT

//Setup事务中wIndex参数取值定义(wIndex字段)=========================================================================
#define  IN_EP1                  0x81        
#define  OUT_EP1                 0x01        
#define  IN_EP2                  0x82
#define  OUT_EP2				 0x01		

//Setup事务中wValue参数取值定义(wValue字段)=========================================================================
#define  DEVICE_REMOTE_WAKEUP    0x01       
#define  ENDPOINT_HALT           0x00      


#endif	//_USB_STAND_DEFINE_H_