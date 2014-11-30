/****************************************Copyright (c)**************************************************
**                              
**                                
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: M25P80.h
**创   建   人: 杨承凯
**创 建 日  期: 2006年8月6日
**最后修改日期: 2007年4月8日
**描        述: Flash源文件，在时钟频率为24.5MHz下测试没问题
********************************************************************************************************/
#ifndef		_MEMORY_NOND_FLASH_H_
#define		_MEMORY_NOND_FLASH_H_

#ifdef		_FLASH_C_
#define		FLASH_EXT
#else
#define		FLASH_EXT	extern
#endif

#define		M25P80			1
#define		M25P32			2

//**************************************************************************
#define		SE		0xD8
#define		BE		0xC7

//***************编译控制**********************************************
#define _FLASH_INIT_
#define _FLASH_WRITE_
#define _FLASH_READ_
#define _FLASH_BUSY_
#define _FLASH_ERASE_
#define _FALSE_OPEN_
#define _FALSE_CLOSE_
//#define _FLASH_TEST_
//#define	_IS_FALSE_OPEN_


//**********************函数声明*********************************************
void FlashInit(void) reentrant;
void FlashOpen(void) reentrant;
void FlashClose(void) reentrant;
void FlashWrite(uint8* pdat,uint32 nStartAddr,uint16 nlen) reentrant;
void FlashRead(uint8* pdat,uint32 nStartAddr,uint16 nlen) reentrant;
void FlashErase(uint32 Addr) reentrant;
BOOL FlashBusy(void) reentrant;
BOOL IsFlashOpen(void) reentrant;
void FlashCheck(void) reentrant;

//端口引脚控制
void FlashPortShut(void) reentrant;
void FlashPortOpen(void) reentrant;
//************************************************************************
#endif	
