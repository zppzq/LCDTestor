/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: SdMmc.h
**��   ��   ��: ��п�(kady1984@163.com)
**�� �� ��  ��: 2008��04��19��
**����޸�����: 2008��04��19��
**��        ��: CF���������ʺ���
*****************************************************************************************************************/
#ifndef _SD_MMC_H_
#define _SD_MMC_H_

/*****************************************************************************************************************
* ��	�ƣ�SdInterfaceInit()
* ��	�ܣ�SD���ӿڳ�ʼ������(SPI��ʼ��)
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void SdInterfaceInit(void) reentrant;


/*****************************************************************************************************************
* ��	�ƣ�SdInit()
* ��	�ܣ�SD����ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void SdInit(void) reentrant; 


/*****************************************************************************************************************
* ��	�ƣ�SdBlockRead()
* ��	�ܣ�SD�����ȡ����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned int SdBlockRead(unsigned long address, unsigned char *pchar) reentrant;


/*****************************************************************************************************************
* ��	�ƣ�SdBlockWrite()
* ��	�ܣ�SD����д�뺯��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned char SdBlockWrite(unsigned long address, unsigned char *wdata) reentrant;

/*****************************************************************************************************************
* ��	�ƣ�SdSectors()
* ��	�ܣ���ȡ��������Ŀ
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned long SdSectors(void) reentrant;

#endif