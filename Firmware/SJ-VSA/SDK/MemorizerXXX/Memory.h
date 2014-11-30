/****************************************Copyright (c)**************************************************
**                              
**                                
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: M25P80.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2006��8��6��
**����޸�����: 2007��4��8��
**��        ��: FlashԴ�ļ�����ʱ��Ƶ��Ϊ24.5MHz�²���û����
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

//***************�������**********************************************
#define _FLASH_INIT_
#define _FLASH_WRITE_
#define _FLASH_READ_
#define _FLASH_BUSY_
#define _FLASH_ERASE_
#define _FALSE_OPEN_
#define _FALSE_CLOSE_
//#define _FLASH_TEST_
//#define	_IS_FALSE_OPEN_


//**********************��������*********************************************
void FlashInit(void) reentrant;
void FlashOpen(void) reentrant;
void FlashClose(void) reentrant;
void FlashWrite(uint8* pdat,uint32 nStartAddr,uint16 nlen) reentrant;
void FlashRead(uint8* pdat,uint32 nStartAddr,uint16 nlen) reentrant;
void FlashErase(uint32 Addr) reentrant;
BOOL FlashBusy(void) reentrant;
BOOL IsFlashOpen(void) reentrant;
void FlashCheck(void) reentrant;

//�˿����ſ���
void FlashPortShut(void) reentrant;
void FlashPortOpen(void) reentrant;
//************************************************************************
#endif	
