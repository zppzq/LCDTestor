/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: SyncTimer.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2011��04��05��
**����޸�����: 2011��04��05��
**��        ��: ͬ����ʱ��ͷ�ļ�
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
