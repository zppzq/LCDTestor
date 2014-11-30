/*
*********************************************************************************************************
*                                               ColBus
*                                               ͨ��Э��
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               ��Ȩ����
*
*
* �ļ��� : ColBusSlaveDescribe.h
* ����   : ��п�(kady1984@hotmail.com)
*********************************************************************************************************
*/
#ifndef _COL_BUS_SLAVE_DESCRIBE_H_
#define _COL_BUS_SLAVE_DESCRIBE_H_


//�豸����ģʽ
#define DEVICE_ENDIAN_MODE		CB_BIG_ENDIAN

//�˵����
#define DEVICE_EP_COUNT			4	

//Э��汾
extern uint8 const nCBVersion[2];

//�豸����������
extern uint8 const arrFactoryInfo[18];

//�豸����
extern uint8 const arrDeviceInfo[14];

//�˵���Ϣ����
extern uint8 const arrEpType[DEVICE_EP_COUNT];		//�˵���������
extern uint8 const arrEpProp[DEVICE_EP_COUNT];		//�˵�����(���ã���̬������)

//�˵�����
extern DT_STORE g_fEpValue[DEVICE_EP_COUNT];
extern void* const arrEpDataPt[DEVICE_EP_COUNT];		//�˵����ݵ�ַ
extern uint8 const arrEpDataLen[DEVICE_EP_COUNT];		//�˵�����ݳ���

//�˵�����λ
extern uint8 const arrUnitEp0[6];
extern void const* const arrEpUnitPt[DEVICE_EP_COUNT];
extern uint8 const arrEpUnitLen[DEVICE_EP_COUNT];

//�����ٽ�������
#define CB_ENTER_CRITICAL()	  	OS_ENTER_CRITICAL()
#define CB_EXIT_CRITICAL()		OS_EXIT_CRITICAL()



#endif //_COL_BUS_SLAVE_DESCRIBE_H_
