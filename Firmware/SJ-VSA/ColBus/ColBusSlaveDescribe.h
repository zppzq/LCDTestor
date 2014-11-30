/*
*********************************************************************************************************
*                                               ColBus
*                                               通信协议
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               版权所有
*
*
* 文件名 : ColBusSlaveDescribe.h
* 作者   : 杨承凯(kady1984@hotmail.com)
*********************************************************************************************************
*/
#ifndef _COL_BUS_SLAVE_DESCRIBE_H_
#define _COL_BUS_SLAVE_DESCRIBE_H_


//设备编译模式
#define DEVICE_ENDIAN_MODE		CB_BIG_ENDIAN

//端点个数
#define DEVICE_EP_COUNT			4	

//协议版本
extern uint8 const nCBVersion[2];

//设备制造商描述
extern uint8 const arrFactoryInfo[18];

//设备描述
extern uint8 const arrDeviceInfo[14];

//端点信息定义
extern uint8 const arrEpType[DEVICE_EP_COUNT];		//端点数据类型
extern uint8 const arrEpProp[DEVICE_EP_COUNT];		//端点属性(启用，静态，输入)

//端点数据
extern DT_STORE g_fEpValue[DEVICE_EP_COUNT];
extern void* const arrEpDataPt[DEVICE_EP_COUNT];		//端点数据地址
extern uint8 const arrEpDataLen[DEVICE_EP_COUNT];		//端点的数据长度

//端点物理单位
extern uint8 const arrUnitEp0[6];
extern void const* const arrEpUnitPt[DEVICE_EP_COUNT];
extern uint8 const arrEpUnitLen[DEVICE_EP_COUNT];

//数据临界区保护
#define CB_ENTER_CRITICAL()	  	OS_ENTER_CRITICAL()
#define CB_EXIT_CRITICAL()		OS_EXIT_CRITICAL()



#endif //_COL_BUS_SLAVE_DESCRIBE_H_
