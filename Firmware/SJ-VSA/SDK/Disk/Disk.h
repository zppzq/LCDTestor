									/****************************************Copyright (c)**************************************************
**                              
**                                
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: M25P80.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2011��3��23��
**����޸�����: 2011��3��23��
**��        ��: ���̹���
********************************************************************************************************/
#ifndef		_DISK_H_
#define		_DISK_H_


//***************�������**********************************************
#define _DISK_INIT_
#define _DISK_WRITE_
#define _DISK_READ_
#define _DISK_BUSY_
#define _DISK_ERASE_
#define _DISK_OPEN_
#define _DISK_CLOSE_






//**********************��������*********************************************
void DiskInit(void);
void DiskProcess(void);
void DiskOpen(void);
void DiskClose(void);
void DiskDisable(void);
void DiskEnable(void);
uint8 DiskRead(OS_EVENT* pPendTask, uint8* pData, uint32 nAddr, uint32 nLen);
uint8 DiskWrite(OS_EVENT* pPendTask, uint8* pData, uint32 nAddr, uint32 nLen);
uint8 DiskErase(OS_EVENT* pPendTask, uint32 nAddr, uint32 nLen);



//�˿����ſ���
void DiskPortShut(void);
void DiskPortOpen(void);
//************************************************************************
#endif	
