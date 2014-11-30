/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: UsbDescriptor.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��04��12��
**����޸�����: 2008��04��12��
**��        ��: USB�����������ļ�
*****************************************************************************************************************/
#include "UsbInclude.h"
#include "UsbDescriptor.h"


//�����궨��============================================================================================

//���������Ͷ���
#define DEVICE_DESCRIPTOR               0x01  //�豸������
#define CONFIGURATION_DESCRIPTOR        0x02  //����������
#define STRING_DESCRIPTOR               0x03  //�ַ���������
#define INTERFACE_DESCRIPTOR            0x04  //�ӿ�������
#define ENDPOINT_DESCRIPTOR             0x05  //�˵�������

//�˵㴫�����Ͷ���
#define ENDPOINT_TYPE_CONTROL           0x00 //���ƴ���
#define ENDPOINT_TYPE_ISOCHRONOUS       0x01 //ͬ������
#define ENDPOINT_TYPE_BULK              0x02 //��������
#define ENDPOINT_TYPE_INTERRUPT         0x03 //�жϴ���



//���岢��ʼ��������====================================================================================

//���豸��������
const CUsbDeviceDesc UsbDeviceDesc =
{ 
	0x12, 								//����������
	0x01, 								//����������
	0x1001, 							//USB�豸�汾��(��BCD���ʾ)
	0x00, 								//USB�豸����룬0x01-0xFEΪ��׼�豸�࣬0xffΪ�����Զ�������
	0x00, 								//������룬ͬ�ϣ�ֵ��USB�涨�ͷ����
	0x00, 								//�豸Э����룬ͬ��
	EP0_PACKET_SIZE, 					//�˵�0�������Ĵ�С
	0xC410, 							//���̱��
	USB_PRODUCT_CLASS_TYPE,				//��Ʒ��ţ������̱��һ�����ʹ�ã�������ע����豸��������Ӧ���������� 
	0x0000, 							//�豸�������(BCD��)
	0x01, 								//���������ַ���������
	0x00, 								//������Ʒ�ַ���������   
	0x03, 								//�����豸���к��ַ���������
	0x01 								//����������������
};

//����������
const CUsbConfigDesc UsbConfigDesc =
{
	0x09, 								//����������
	0x02, 								//����������
	0x2000, 							//���������ص����������Ĵ�С
	0x01, 								//��������֧�ֵĽӿ�����
	0x01, 								//�����õı��
	0x00, 								//���������õ��ַ���������ֵ
	0x80, 								//���߹��磬��֧��Զ�̻���
	0x0F 								//�豸��������ȡ��������(�˴�Ϊ30mA)
};

//�ӿ�������
const CUsbInterfaceDesc UsbInterfaceDesc =
{
	0x09, 								//����������
	0x04, 								//����������
	0x00, 								//�ӿڵı��
	0x00, 								//���õĽӿڱ��
	0x02, 								//�ýӿ�ʹ�ö˵������������˵�0
	USB_INTERFACE_TRANS_TYPE, 			//�ӿ�����(0x08����MASS STORAGE DEVICE) 
	USB_INTERFACE_TRANS_SUB_TYPE, 		//�ӿ�������(0x06����SCSI Transparent command set�����򻯿�����)
	USB_INTERFACE_TRANS_PROTACAL, 		//�ӿ�����ѭ��Э��(0x50���� BULK-ONLY Э�飬������������Э��)
	0x00 								//�����ýӿڵ��ַ�������ֵ
};

//�˵�1������
const CUsbEndpointDesc UsbEndpoint1Desc =
{
	0x07, 								//����������
	0x05,			 					//����������
	0x81, 								//�˵��ַ�������������(IN)
	USB_EP_TRANS_TYPE, 					//�˵�Ĵ�����������
	SWAP_WORD(EP1_PACKET_SIZE), 		//�˵��ա����������Ĵ�С(LITTLE ENDIAN)
	USB_EP_SCAN_TIME 					//������ѯ�˵��ʱ����(����)
}; 

//�˵�2������
const CUsbEndpointDesc UsbEndpoint2Desc =
{
	0x07, 								//����������
	0x05, 								//����������
	0x02, 								//�˵��ַ�������������(OUT)
	USB_EP_TRANS_TYPE, 					//�˵�Ĵ�����������
	SWAP_WORD(EP2_PACKET_SIZE), 		//�˵��ա����������Ĵ�С(LITTLE ENDIAN)
	USB_EP_SCAN_TIME 					//������ѯ�˵��ʱ����(����)
}; 


//������Ʒ���ַ���
#define STR0LEN 4
code const BYTE UsbProductStringDesc[STR0LEN] =
{
	STR0LEN, 0x03, 
	0x09, 0x04
};

//�������̵��ַ���
#define STR1LEN sizeof("KadyUsbDevice")*2
code const BYTE UsbCompanyStringDesc[STR1LEN] =
{
	STR1LEN, 0x03,
	'K', 0,
	'a', 0,
	'd', 0,
	'y', 0,
	'U', 0,
	's', 0,
	'b', 0,
	'D', 0,
	'e', 0,
	'v', 0,
	'i', 0,
	'c', 0,
	'e', 0,
};

//�豸�����ַ���
#define STR2LEN sizeof("UsbCommDevice")*2
code const BYTE UsbDeviceStringDesc[STR2LEN] =
{
	STR2LEN, 0x03,
	'U', 0,
	's', 0,
	'b', 0,
	'C', 0,
	'o', 0,
	'm', 0,
	'm', 0,
	'D', 0,
	'e', 0,
	'v', 0,
	'i', 0,
	'c', 0,
	'e', 0,
};

//�����豸���к��ַ���������
#define STR3LEN sizeof("0079876543210")*2
code const BYTE UsbSerialStringDesc[STR3LEN] =
{
	STR3LEN, 0x03,
	'0', 0,
	'0', 0,
	'7', 0,
	'9', 0,
	'8', 0,
	'7', 0,
	'6', 0,
	'5', 0,
	'4', 0,
	'3', 0,
	'2', 0,
	'1', 0,
	'0', 0
};

//�����ַ���
BYTE* const UsbStringDescTable[] = {UsbProductStringDesc, UsbCompanyStringDesc, UsbDeviceStringDesc, UsbSerialStringDesc};





