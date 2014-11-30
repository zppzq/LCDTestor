/****************************************Copyright (c)**************************************************
**                              
**                                
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Memorizer.h
**创   建   人: 罗仕强
**创 建 日  期: 2009年4月14日
**最后修改日期: 2009年4月14日
**描        述: 存储器接口函数
********************************************************************************************************/
#ifndef		_MEMORIZER_INTERFACE_H_
#define		_MEMORIZER_INTERFACE_H_

#include "Memory.h"

//选择FLASH种类======================================
#define	FLASH_TYPE		K9F1G08U0A
#define	DATALEN		NAND_PAGE_SIZE
extern uint8 xdata TxBuff[DATALEN];
extern uint8 xdata RxBuff[DATALEN];


//函数定义==========================================
//存储器初始化
#define	MemPorInit()	FlashInit()

//开存储器电源
#define	MemOpen()		FlashOpen()

//关存储器电源
#define	MemClose()		FlashClose()

//读存储器
#define	MemRead(x,y,z)	FlashRead(x,y,z)

//写存储器
#define	MemWrite(x,y,z)	FlashWrite(x,y,z)

//擦除存储器
#define	MemErase(x)	FlashErase(x)

#define	MemTest()		FlashTest()

#define	MemBusy()		FlashBusy()
//************************************************************************
#endif	
