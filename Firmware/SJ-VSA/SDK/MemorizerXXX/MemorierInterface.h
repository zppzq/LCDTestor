/****************************************Copyright (c)**************************************************
**                              
**                                
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Memorizer.h
**��   ��   ��: ����ǿ
**�� �� ��  ��: 2009��4��14��
**����޸�����: 2009��4��14��
**��        ��: �洢���ӿں���
********************************************************************************************************/
#ifndef		_MEMORIZER_INTERFACE_H_
#define		_MEMORIZER_INTERFACE_H_

#include "Memory.h"

//ѡ��FLASH����======================================
#define	FLASH_TYPE		K9F1G08U0A
#define	DATALEN		NAND_PAGE_SIZE
extern uint8 xdata TxBuff[DATALEN];
extern uint8 xdata RxBuff[DATALEN];


//��������==========================================
//�洢����ʼ��
#define	MemPorInit()	FlashInit()

//���洢����Դ
#define	MemOpen()		FlashOpen()

//�ش洢����Դ
#define	MemClose()		FlashClose()

//���洢��
#define	MemRead(x,y,z)	FlashRead(x,y,z)

//д�洢��
#define	MemWrite(x,y,z)	FlashWrite(x,y,z)

//�����洢��
#define	MemErase(x)	FlashErase(x)

#define	MemTest()		FlashTest()

#define	MemBusy()		FlashBusy()
//************************************************************************
#endif	
