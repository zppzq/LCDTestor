/*
*********************************************************************************************************
*                                               ColBus
*                                               ͨ��Э��
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               ��Ȩ����
*
*
* �ļ��� : ColBusDef.h
* ����   : ��п�(kady1984@hotmail.com)
*********************************************************************************************************
*/

#ifndef _COLBUS_DEF_H_
#define _COLBUS_DEF_H_

//�������Ͷ���
//typedef unsigned char uint8;
//typedef char int8;
//typedef unsigned short uint16;
//typedef short int16;

#include "includes.h"

#ifndef TRUE
#define TRUE		1
#endif

#ifndef FALSE
#define FALSE		0
#endif

//�����ֶ���
#define CODM_MASK			0xE0
#define CODM_MAC			0x20
#define CODM_NET			0x40

//������
#define MTS_MASK			0x10
#define MTS_DOWN			0x10
#define MTS_UP				0x00

//��ַ��Ϣ
#define CB_ADDR_SIZE_MASK	0x0F 



//MACָ�
#define CB_SET_ADDR			0x01


//����ָ�����
#define CB_RESET			0x11		//�豸��λָ��
#define CB_VERSION			0x12		//��ȡЭ��汾
#define CB_EP_DATA_GETC		0x05		//��ȡ����
#define CB_EP_DATA_SETC		0x15		//��������
#define CB_EP_DATA_GETS		0x06		//��ȡ����
#define CB_EP_DATA_SETS		0x16		//��������
#define CB_EP_DATA_PREPC	0x17		//׼���˵�����
#define CB_EP_DATA_PREPS	0x18		//׼���˵�����

#define CB_SAMPLE_SYNC		0x35		//ͬ�����ݲɼ�


#define CB_SAMPLE_START		0x41		//�������ݲɼ�
#define CB_SAMPLE_PAUSE		0x42		//�������ݲɼ�
#define CB_SAMPLE_RESUME	0x43		//�������ݲɼ�
#define CB_SAMPLE_STOP		0x44		//ֹͣ���ݲɼ�
#define CB_DEVICE_CFG		0x45		//�豸����
#define CB_DEVICE_PARA		0x46		//�豸����
#define CB_EP_CFG			0x47		//�˵�����
#define CB_ZERO				0x48		//����
#define CB_EP_MEM			0x49		//���ʶ˵�洢��


#define CB_COMM_CFG			0x1A		//ͨ������
#define CB_REG				0x1F
#define CB_COMM_WAKE		0x6A		//���绽��
#define CB_ACT				0x6B		//Э�鼤��


//��ָ��
//CB_SAMPLE_START��CB_SAMPLE_STOP
#define CB_OP_STATIC				0x41
#define CB_OP_STATICS				0x42
#define CB_OP_DYNAMIC				0x81
#define CB_OP_SYNC					0x82



//CB_DEVICE_CFG
#define CB_DEVICE_MANUFACTURER		0x11
#define CB_DEVICE_MODEL				0x12
#define CB_DEVICE_EP_NUM			0x13
#define CB_DEVICE_ENDIAN			0x14
#define CB_TIME_SET					0x9A
#define CB_TIME_GET					0x1A


//CB_DEVICE_PARA
#define CB_DEVICE_PARA_NUM			0x11
#define CB_DEVICE_PARA_GETC			0x12
#define CB_DEVICE_PARA_SETC			0x92

//CB_EP_CFG
#define CB_EP_TYPE_GETC				0x11
#define CB_EP_TYPE_GETS				0x51
#define CB_EP_PROP_GETC				0x12
#define CB_EP_PROP_GETS				0x52
#define CB_EP_UNIT_GETC				0x13
#define CB_EP_UNIT_GETS				0x53
#define CB_EP_PARA_SETC				0x94
#define CB_EP_PARA_GETC				0x14
#define CB_EP_CALI_SETC				0x95
#define CB_EP_CALI_REFC				0x96
#define CB_EP_CALI_RSTC				0x97 
#define CB_EP_CALI_GETC				0x15

//CB_ZERO
#define CB_ZERO_SETC				0x11
#define CB_ZERO_SETS				0x51
#define CB_ZERO_CANCELC				0x12
#define CB_ZERO_CANCELS				0x52

//CB_EP_MEM
#define CB_EP_MEM_SEG_NUM			0x11
#define CB_EP_MEM_SEG_LEN			0x12
#define CB_EP_MEM_SEG_SEL			0x93
#define CB_EP_MEM_SEG_INDEX			0x14
#define CB_EP_MEM_SEG_DATA			0x15
#define CB_EP_MEM_SEG_ERASE			0x95
#define CB_EP_MEM_SEG_CLEAR			0x96
#define CB_EP_MEM_SEG_DEL			0x97
#define CB_EP_MEM_CLEAR				0x98

//CB_COMM_CFG
#define CB_COMM_CHECK				0x11
#define CB_COMM_BUFFLEN_GET			0x12
#define CB_COMM_BEAT_SET			0x93
#define CB_COMM_BEAT_GET			0x13
#define CB_COMM_RATE_SET			0x94
#define CB_COMM_CHANNEL_SET			0x95
#define CB_COMM_QUALITY_TEST		0x96
#define CB_COMM_ENCR_SET			0x1A
#define CB_COMM_ENCR_CLEAR			0x9A

//ͬ����־
#define CB_SYNC_CALI_TIME			0x01		//У׼ʱ��
#define CB_SYNC_SAMPLE_REF			0x02		//ʹ���˲ο�Ƶ��
#define CB_SYNC_READ_DATA			0x04		//������
#define CB_SYNC_RETURN_EMPTY		0x08		//�ظ����ݿ�
#define CB_SYNC_DATALEN_UINT16		0x10		//�ظ����ݳ���Ϊ2�ֽ�

//����Ӧ�����
#define CB_ERR_BUSY					0xA1
#define CB_ERR_LOPOWER				0xA2
#define CB_ERR_CMD_US				0xA3
#define CB_ERR_SUBCMD_US			0xA4
#define CB_ERR_DATA					0xA5
#define CB_ERR_SEG_TYPE				0xA6
#define CB_ERR_SEG_PARA				0xA7
#define CB_ERR_SEG_ADDR				0xA8
#define CB_ERR_EP_ADDR				0xA9
#define CB_ERR_KEY					0xAA
#define CB_ERR_MODEL				0xAB
#define CB_ERR_UNKNOWN				0xFF

//����ģʽ
#define CB_LITTLE_ENDIAN			0x85
#define CB_BIG_ENDIAN				0x8C

#endif //_COLBUS_DEF_H_


