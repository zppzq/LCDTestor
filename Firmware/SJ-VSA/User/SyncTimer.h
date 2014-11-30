/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: SyncTimer.h
**创   建   人: 杨承凯
**创 建 日  期: 2011年04月05日
**最后修改日期: 2011年04月05日
**描        述: 同步定时器头文件
*****************************************************************************************************************/
#ifndef _SYNC_TIMER_H_
#define _SYNC_TIMER_H_

void SyncTimerInit(void);
void SyncTimerStop(void);
void SetSyncTimerRate(int nRate);
void SyncTimerResetRun(void);
void OpenSyncTimerISR(void);
void CloseSyncTimerISR(void);	
void SyncTimerCaliAndRun(void);


#endif
