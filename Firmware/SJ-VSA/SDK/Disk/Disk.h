									/****************************************Copyright (c)**************************************************
**                              
**                                
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: M25P80.h
**创   建   人: 杨承凯
**创 建 日  期: 2011年3月23日
**最后修改日期: 2011年3月23日
**描        述: 磁盘管理
********************************************************************************************************/
#ifndef		_DISK_H_
#define		_DISK_H_


//***************编译控制**********************************************
#define _DISK_INIT_
#define _DISK_WRITE_
#define _DISK_READ_
#define _DISK_BUSY_
#define _DISK_ERASE_
#define _DISK_OPEN_
#define _DISK_CLOSE_






//**********************函数声明*********************************************
void DiskInit(void);
void DiskProcess(void);
void DiskOpen(void);
void DiskClose(void);
void DiskDisable(void);
void DiskEnable(void);
uint8 DiskRead(OS_EVENT* pPendTask, uint8* pData, uint32 nAddr, uint32 nLen);
uint8 DiskWrite(OS_EVENT* pPendTask, uint8* pData, uint32 nAddr, uint32 nLen);
uint8 DiskErase(OS_EVENT* pPendTask, uint32 nAddr, uint32 nLen);



//端口引脚控制
void DiskPortShut(void);
void DiskPortOpen(void);
//************************************************************************
#endif	
