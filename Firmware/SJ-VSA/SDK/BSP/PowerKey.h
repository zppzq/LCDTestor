/****************************************Copyright (c)************************************************************
**                              
**                         			�����������Ƽ����޹�˾
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: PowerKey.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2011��1��5��
**����޸�����: 2011��1��5��
**��        ��: ���ؿ���
*****************************************************************************************************************/
#ifndef 	_POWER_KEY_H_
#define 	_POWER_KEY_H_


//��Դ���ؿ��ƽ�
#define	POWER_CTR_PORT		GPIOC
#define	POWER_CTR			3

//��Դ���ؼ���
#define	POWER_KEY_PORT		GPIOC
#define	POWER_KEY			2



void PowerKeyInit(void) reentrant;
void PowerDown(void) reentrant;
void PowerEventClear(void) reentrant;
void PowerKeyVarInit(void) reentrant;

#endif


