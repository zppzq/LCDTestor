/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: UsbComm.c
**��   ��   ��: ��п�(kady1984@163.com)
**�� �� ��  ��: 2008��04��17��
**����޸�����: 2008��04��17��
**��        ��: USB�жϴ�����غ���
*****************************************************************************************************************/
#include <stdio.h>
#include "UsbInclude.h"
#include "c8051f340.h"
#include "F34xUsbRegister.h"
#include "UsbStandDefine.h"
#include "UsbDescriptor.h"
#include "..\OsCommon\Includes.h"

//��������
BYTE code USB_ONES_PACKET[2] = {0x01, 0x00}; 		//USB DATA1���ݰ�
BYTE code USB_ZERO_PACKET[2] = {0x00, 0x00}; 		//USB0 DATA0���ݰ�

//�ڲ�ȫ�ֱ�������
static xdata CUsbSetupCmd Setup; 									//�豸���󻺳���
static xdata BYTE UsbState; 										//��¼��ǰUSB��״̬
static xdata unsigned int UsbDataSize; 								//���ݳ���
static xdata unsigned int UsbDataSent; 								//�Ѿ����͹�������
static xdata BYTE* UsbDataPtr; 										//����ָ��
static xdata BYTE UsbEpStatus[3] = { EP_IDLE, EP_IDLE, EP_IDLE };	//ÿ���˵��״̬

//OUT���ݻ�����
unsigned xdata UsbOutCount;								
unsigned char xdata UsbOutBuffer[EP2_PACKET_SIZE];

//IN���ݻ�����
unsigned xdata bUsbInNeed;
unsigned xdata UsbInCount;
unsigned char xdata UsbInBuffer[EP1_PACKET_SIZE];

//�ź�������
OS_EVENT *pUsbRecvEvent;
OS_EVENT *pUsbSendEvent;
unsigned char UsbErr;


//��������========================================================================================================

/*****************************************************************************************************************
* ��	�ƣ�Usb0Init()
* ��	�ܣ�USB0��ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void Usb0Init(void) reentrant
{
	UsbWriteByte(POWER, 0x08); 			//��λUSB
	UsbWriteByte(IN1IE, 0x07); 			//ʹ��USB�˵�0��1��2�������ж�
	UsbWriteByte(OUT1IE,0x07); 			//ʹ��USB�˵�0��1��2������ж�
	UsbWriteByte(CMIE,  0x07); 			//ʹ��USB��λ����ͣ���ָ����ж�
	EIE1 |= 0x02; 						//ʹ��USB���ж�
}

/*****************************************************************************************************************
* ��	�ƣ�Usb0Open()
* ��	�ܣ�����USB0
* ��ڲ�������
* ���ڲ�������
* ˵	����Ҫ�����������һ��ʼ���ã������޷���ʱ�ظ������������ź�
*****************************************************************************************************************/
void Usb0Open() reentrant
{
	pUsbRecvEvent = OSSemCreate(0);		//�����������ݵ��ź���
	pUsbSendEvent = OSSemCreate(0);		//�����������ݵ��ź���

	USB0XCN = 0xE0; 					//ʹ��USB�˿���������ȫ��ģʽ
	UsbWriteByte(CLKREC, 0x80); 		//ʹ��ʱ�ӻָ�
	UsbWriteByte(POWER, 0x01); 			//����USB��ͬʱʹ�����߹�����
}

/*****************************************************************************************************************
* ��	�ƣ�Usb0VariInit()
* ��	�ܣ�USB0������ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void Usb0VariInit(void) reentrant
{
	UsbInCount=0;
	UsbOutCount=0;
	bUsbInNeed = FALSE;

	pUsbRecvEvent = NULL;
	pUsbSendEvent = NULL;
}

/*****************************************************************************************************************
* ��	�ƣ�UsbFifoRead()
* ��	�ܣ���FIFO���ж�ȡ����
* ��ڲ�����nAddr - �˵����ƣ�pDat - ���ջ�����ָ�룻nLen - ���ݸ���
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void UsbFifoRead(BYTE nAddr, BYTE *pDat, unsigned int nLen) reentrant
{
	int i;

	EA = 0;

	if (nLen > 0)
	{
		while(USB0ADR & 0x80);				//�ȴ�ֱ������
		USB0ADR = nAddr; 					//���õ�ַ
		USB0ADR |= 0xC0; 					//�����Զ���ʼ�����ݶ�����
  
		//��������
		for (i=0; i<nLen; i++)
		{
			while(USB0ADR & 0x80);			//�ȴ�����׼����
			pDat[i] = USB0DAT; 				//�������ݵ����������
		}

		USB0ADR = 0;						//���ݶ�ȡ��ϣ�����Զ���ʼ�����ݶ�������־
	}

	EA = 1;
}

/*****************************************************************************************************************
* ��	�ƣ�UsbFifoWrite()
* ��	�ܣ���FIFO����д������
* ��ڲ�����nAddr - �˵����ƣ�pDat - ���ջ�����ָ�룻nLen - ���ݸ���
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void UsbFifoWrite(BYTE nAddr, BYTE *pDat, unsigned int nLen) reentrant
{
	int i;

	EA = 0;

	if (nLen > 0)
	{
		while(USB0ADR & 0x80);				//�ȴ�ֱ������
		USB0ADR = nAddr; 					//���õ�ַ
  
		//��������
		for (i=0; i<nLen; i++)
		{
			while(USB0ADR & 0x80);			//�ȴ�����׼����
			USB0DAT = pDat[i]; 				//�������ݵ�FIFO����
		}
	}

	EA = 1;
}

/*****************************************************************************************************************
* ��	�ƣ�Usb0DeviceRecv()
* ��	�ܣ�USB0�豸�˽��պ���
* ��ڲ�����pBuff - ������ָ�룬nLen - ��Ҫ���յ����ݸ�����nTimeOut - ���ճ�ʱʱ��
* ���ڲ�����ʵ�ʽ��յ������ݸ���
* ˵	������
*****************************************************************************************************************/
#ifdef _USB0_DEVICE_RECV_
int Usb0DeviceRecv(char* pBuff, int nLen, unsigned int nTimeOut) reentrant
{
	unsigned int nRealLen = 0;
	
	//�ȴ������¼�
	OSSemPend(pUsbRecvEvent, nTimeOut, &UsbErr);
	
	if(UsbErr == OS_NO_ERR)
	{
		OS_ENTER_CRITICAL();

#ifdef USB_USE_MEM_BUFFERD		//���ʹ�����ڴ滺�巽ʽ
		nRealLen = (nLen > UsbOutCount) ? UsbOutCount : nLen;						//ȷ����Ч���ݳ���
		memcpy(pBuff, UsbOutBuffer, nRealLen);										//��������
		UsbOutCount = 0;															

#else	//���û��ʹ���ڴ滺��
		
		//ѡ��˵�2
		UsbWriteByte(INDEX, EP2_OUT_IDX);
		
		//��ȡ���ݵ��û�������
		nRealLen = (nLen > UsbOutCount) ? UsbOutCount : nLen;						//ȷ����Ч���ݳ���
		UsbFifoRead(FIFO_EP2, pBuff, nRealLen); 									//��ȡ���û�������

		//��δ��ȡ������ݶ�ȡ�꣬�����´ζ�����ʱ����
		if(UsbOutCount - nRealLen > 0)
		{
			//Ϊ�˱����û�ʹ�õ�pBuff����UsbOutBuffer���������Խ�UsbOutBufferָ�����nRealLen
			UsbFifoRead(FIFO_EP2, UsbOutBuffer + nRealLen, UsbOutCount - nRealLen); //��ȡ���ڴ�	
		}

		//���������Ͷ�ȡȷ��
		UsbWriteByte(EOUTCSR1, 0);
		UsbOutCount = 0;
#endif

		OS_EXIT_CRITICAL();
	}

	return nRealLen;
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�Usb0DeviceSend()
* ��	�ܣ�USB0�豸�˽��պ���
* ��ڲ�����pBuff - ������ָ�룬nLen - ��Ҫ���͵����ݸ�����nTimeOut - ���ͳ�ʱʱ��
* ���ڲ�����ʵ�ʷ��͵����ݸ���
* ˵	������
*****************************************************************************************************************/
#ifdef _USB0_DEVICE_SEND_
int Usb0DeviceSend(char* pBuff, int nLen, unsigned int nTimeOut) reentrant
{
	unsigned int nRealLen = 0;

	//�����ٽ���
	OS_ENTER_CRITICAL();

	//�ж���ǰ�������Ƿ��Ѿ�����
	if(bUsbInNeed == TRUE)
	{
		OS_EXIT_CRITICAL();
		return -1;	
	}

	//�����ݻ�����д������	
	nRealLen = (nLen > EP1_PACKET_SIZE) ? EP1_PACKET_SIZE : nLen;
	UsbWriteByte(INDEX, EP1_IN_IDX); 			//���ö˵�1����
	UsbFifoWrite(FIFO_EP1, pBuff, nRealLen);	//��In FIFOд������
	UsbWriteByte(EINCSR1, rbInINPRDY);			//��������׼���ñ�־
	bUsbInNeed = TRUE;							//�������������־

	//�˳��ٽ���
	OS_EXIT_CRITICAL();

	//�ȴ��������¼�
	OSSemPend(pUsbSendEvent, nTimeOut, &UsbErr);

	//���سɹ���ʧ����Ϣ
	if(UsbErr == OS_NO_ERR) return nRealLen;
	return 0;
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�UsbForceStall()
* ��	�ܣ�ǿ����ֹUSB�˵�0ͨ��
* ��ڲ�������
* ���ڲ�������
* ˵	����ͨ��Э�鲻����ͨ���쳣ʱ��ǿ��ֹͣͨ��
*****************************************************************************************************************/
void UsbForceStall(void) reentrant
{
	UsbWriteByte(INDEX, EP0_IDX);			//ѡ��˵�0
	UsbWriteByte(E0CSR, rbSDSTL); 			//����ǿ��ֹͣλ
	UsbEpStatus[0] = EP_STALL; 				//���˵�0����Ϊǿ��ֹͣ״̬
}

/*****************************************************************************************************************
* ��	�ƣ�UsbHandleIn()
* ��	�ܣ�USB In������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/ 
void UsbHandleIn() reentrant
{
	BYTE control_reg;

	UsbWriteByte(INDEX, EP1_IN_IDX); 			//���ö˵�1����
	UsbReadByte(EINCSR1, control_reg); 			//��ȡ�˵�1�Ŀ��ƼĴ���

	if (UsbEpStatus[1] == EP_HALT) 				//����˵�0��������ǿ���˳��ж�
	{
		UsbWriteByte(EINCSR1, rbInSDSTL);
	}

	//��������������
	else
	{
		if(control_reg & rbInSTSTL) 			//����ϴ����ݷ���δ�ɹ��������־
		{
			UsbWriteByte(EINCSR1, rbInCLRDT);
		}

		if(control_reg & rbInUNDRUN) 			//����ϴ�����û�з����źţ������־
		{
			UsbWriteByte(EINCSR1, 0x00);
		}

		//���������ѷ����ź�
		if(bUsbInNeed == TRUE)
		{
			bUsbInNeed = FALSE;
			OSSemPost(pUsbSendEvent);
		}
	}
}

/*****************************************************************************************************************
* ��	�ƣ�UsbHandleOut()
* ��	�ܣ�USB���ݽ��մ�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void UsbHandleOut() reentrant
{
	BYTE count=0;
	BYTE control_reg;

	UsbWriteByte(INDEX, EP2_OUT_IDX);
	UsbReadByte(EOUTCSR1, control_reg);

	if (UsbEpStatus[2] == EP_HALT)
	{
		UsbWriteByte(EOUTCSR1, rbOutSDSTL);		//״̬����ǿ����ֹͨ��
	}

	//��ȡ����
	else
	{
		if (control_reg & rbOutSTSTL)
		{
			UsbWriteByte(EOUTCSR1, rbOutCLRDT);
		}

		//��ȡ�յ������ݳ���
		UsbReadByte(EOUTCNTL, count);
		UsbOutCount = count;
		UsbReadByte(EOUTCNTH, count);
		UsbOutCount |= ((unsigned)count)<<8;

		//������ݳ���
		if(UsbOutCount > EP2_PACKET_SIZE) UsbOutCount = EP2_PACKET_SIZE;

#ifdef USB_USE_MEM_BUFFERD
		//���ʹ���ڴ滺�巽ʽ�����ȡ���ڴ�
		UsbFifoRead(FIFO_EP2, UsbOutBuffer, UsbOutCount); 		//��ȡ���ڴ�
		UsbWriteByte(EOUTCSR1, 0);								//���������Ͷ�ȡȷ��
#endif
		//�����յ������ź�
		OSSemPost(pUsbRecvEvent);
	}
}

/*****************************************************************************************************************
* ��	�ƣ�UsbGetStatus()
* ��	�ܣ�USB״̬��ȡ����
* ��ڲ�������
* ���ڲ�������
* ˵	����USB��׼������
*****************************************************************************************************************/
void UsbGetStatus(void) reentrant
{
	//�ж��ֶ��Ƿ���Ч
	if (Setup.wValue.c[MSB] || Setup.wValue.c[LSB] || Setup.wLength.c[MSB] || (Setup.wLength.c[LSB] != 2))
	{
		UsbForceStall();
	}

	switch (Setup.bmRequestType)
	{
		//��������Ϊ�豸
		case OUT_DEVICE:
			if (Setup.wIndex.c[MSB] || Setup.wIndex.c[LSB])
			{
				UsbForceStall(); 	//������Ч��ֹͣͨ��
			}
			else
			{
				//�ظ����ֽ�0x00
				UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
				UsbDataSize = 2; 
			}
			break;
	
		//��������Ϊ�ӿ�
		case OUT_INTERFACE:
			if ((UsbState != DEV_CONFIGURED) || Setup.wIndex.c[MSB] || Setup.wIndex.c[LSB])
			{
				UsbForceStall(); 	//������Ч��ֹͣͨ��
			}
			else
			{
				//�ظ����ֽ�0x00
				UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
				UsbDataSize = 2;
			}
			break;
	
		//��������Ϊ�˵�
		case OUT_ENDPOINT:
			if ((UsbState != DEV_CONFIGURED) || Setup.wIndex.c[MSB])
			{
				UsbForceStall();  	//������Ч��ֹͣͨ��
			}
			else
			{
				//���ָ��˵�1
				if (Setup.wIndex.c[LSB] == IN_EP1)
				{
					if (UsbEpStatus[1] == EP_HALT)
					{
					 	//����˵�1�Ǵ���״̬���ظ�0x01,0x00
						UsbDataPtr = (BYTE*)&USB_ONES_PACKET;
						UsbDataSize = 2;
					}
					else
					{
						//����ͻظ�0x00,0x00��ָʾ�˵��Ѽ���
						UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
						UsbDataSize = 2;
					}
				}
				else	//���ָ��������Ķ˵�
				{
					//���ָ��˵�2
					if (Setup.wIndex.c[LSB] == OUT_EP2)
					{
						//����˵�2�Ǵ���״̬���ظ�0x01,0x00
						if (UsbEpStatus[2] == EP_HALT)
						{
							UsbDataPtr = (BYTE*)&USB_ONES_PACKET;
							UsbDataSize = 2;
						}
						else
						{
							//����ͻظ�0x00,0x00��ָʾ�˵��Ѽ���
							UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
							UsbDataSize = 2;
						}
					}
					else
					{
						UsbForceStall(); 	//������Ч��ֹͣͨ��
					}
				}
			}
			break;
	
		//��Ч�ı�������
		default:
			UsbForceStall();
			break;
	}

	//���ͨ����������(û�е��ù�UsbForceStall()����)
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, rbSOPRDY); 	//��Ӧ����Ӳ��������OPRDYλ                  
		UsbEpStatus[0] = EP_TX; 			//���˵���״̬��Ϊ����״̬
		UsbDataSent = 0;					//��������������
	}
}

/*****************************************************************************************************************
* ��	�ƣ�UsbClearFeature()
* ��	�ܣ�USB�����������
* ��ڲ�������
* ���ڲ�������
* ˵	����USB��׼������
*****************************************************************************************************************/
void UsbClearFeature() reentrant
{
	if ((UsbState != DEV_CONFIGURED) 
		|| (Setup.bmRequestType == IN_DEVICE) 
		|| (Setup.bmRequestType == IN_INTERFACE) 
		|| Setup.wValue.c[MSB] 
		|| Setup.wIndex.c[MSB]
		|| Setup.wLength.c[MSB] 
		|| Setup.wLength.c[LSB])
	{
		//����������ԣ�ǿ��ֹͣͨ��
		UsbForceStall();
	}

	else
	{
		if( (Setup.bmRequestType == IN_ENDPOINT) &&			
			(Setup.wValue.c[LSB] == ENDPOINT_HALT) &&		
			((Setup.wIndex.c[LSB] == IN_EP1) || 			
			(Setup.wIndex.c[LSB] == OUT_EP2))) 				
		{
			if(Setup.wIndex.c[LSB] == IN_EP1)
			{
				UsbWriteByte(INDEX, EP1_IN_IDX); 				
				UsbWriteByte(EINCSR1, rbInCLRDT);
				UsbEpStatus[1] = EP_IDLE; 						                   
			}
			else
			{
				UsbWriteByte(INDEX, EP2_OUT_IDX); 				
				UsbWriteByte(EOUTCSR1, rbOutCLRDT);
				UsbEpStatus[2] = EP_IDLE; 						
			}
		}
		else
		{
			UsbForceStall(); 								
		}
	}
	
	//����Ӧ��
	UsbWriteByte(INDEX, EP0_IDX); 								
	if(UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}


/*****************************************************************************************************************
* ��	�ƣ�UsbSetFeature()
* ��	�ܣ�USB�������ú���
* ��ڲ�������
* ���ڲ�������
* ˵	����USB��׼������
*****************************************************************************************************************/
void UsbSetFeature(void) reentrant
{
	if ((UsbState != DEV_CONFIGURED) ||
			(Setup.bmRequestType == IN_DEVICE) ||
			(Setup.bmRequestType == IN_INTERFACE) ||
			Setup.wValue.c[MSB] || Setup.wIndex.c[MSB]|| Setup.wLength.c[MSB]
			|| Setup.wLength.c[LSB])
	{
		UsbForceStall();
	}


	else
	{
		if ((Setup.bmRequestType == IN_ENDPOINT)&&
			(Setup.wValue.c[LSB] == ENDPOINT_HALT) &&
			((Setup.wIndex.c[LSB] == IN_EP1) || (Setup.wIndex.c[LSB] == OUT_EP2)))
		{
			if (Setup.wIndex.c[LSB] == IN_EP1)
			{
				UsbWriteByte(INDEX, EP1_IN_IDX);
				UsbWriteByte(EINCSR1, rbInSDSTL);
				UsbEpStatus[1] = EP_HALT;
			}
			else
			{
				UsbWriteByte(INDEX, EP2_OUT_IDX);
				UsbWriteByte(EOUTCSR1, rbOutSDSTL);
				UsbEpStatus[2] = EP_HALT;
			}
		}
		else
		{
			UsbForceStall();
		}
	}

	//����Ӧ��
	UsbWriteByte(INDEX, EP0_IDX);
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}

/*****************************************************************************************************************
* ��	�ƣ�UsbSetAddress()
* ��	�ܣ�USB�豸��ַ���ú���
* ��ڲ�������
* ���ڲ�������
* ˵	����USB��׼������
*****************************************************************************************************************/
void UsbSetAddress(void) reentrant
{
	if ((Setup.bmRequestType != IN_DEVICE) ||
			Setup.wIndex.c[MSB]  || Setup.wIndex.c[LSB] ||
			Setup.wLength.c[MSB] || Setup.wLength.c[LSB]|| 
			Setup.wValue.c[MSB]  || (Setup.wValue.c[LSB] & 0x80))
	{
		UsbForceStall();
	}

	UsbEpStatus[0] = EP_ADDRESS;
	if(Setup.wValue.c[LSB] != 0)
	{
		UsbState = DEV_ADDRESS;
	}
	else
	{
		UsbState = DEV_DEFAULT;
	}

	//����Ӧ��
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}


/*****************************************************************************************************************
* ��	�ƣ�UsbGetDescriptor()
* ��	�ܣ�USB��������ȡ����
* ��ڲ�������
* ���ڲ�������
* ˵	����USB��׼������
*****************************************************************************************************************/
void UsbGetDescriptor(void) reentrant
{
	switch (Setup.wValue.c[MSB])
	{ 
		case DSC_DEVICE:
			UsbDataPtr = (BYTE*) &UsbDeviceDesc;
			UsbDataSize = UsbDeviceDesc.bLength;
			break;
	
		case DSC_CONFIG:
			UsbDataPtr = (BYTE*) &UsbConfigDesc;
			UsbDataSize = UsbConfigDesc.wTotalLength.c[MSB] + 256*UsbConfigDesc.wTotalLength.c[LSB];
			break;
	
		case DSC_STRING:
			UsbDataPtr = UsbStringDescTable[Setup.wValue.c[LSB]];
			UsbDataSize = *UsbDataPtr;
			break;
	
		case DSC_INTERFACE:
			UsbDataPtr = (BYTE*) &UsbInterfaceDesc;
			UsbDataSize = UsbInterfaceDesc.bLength;
			break;
	
		case DSC_ENDPOINT:
			if((Setup.wValue.c[LSB] == IN_EP1) || (Setup.wValue.c[LSB] == OUT_EP2))
			{
				if(Setup.wValue.c[LSB] == IN_EP1)
				{
					UsbDataPtr = (BYTE*) &UsbEndpoint1Desc;
					UsbDataSize = UsbEndpoint1Desc.bLength;
				}
				else
				{
					UsbDataPtr = (BYTE*) &UsbEndpoint2Desc;
					UsbDataSize = UsbEndpoint2Desc.bLength;
				}
			}
			else
			{
				UsbForceStall();
			}
			break;
	
		default:
			UsbForceStall();
			break;
	}

	if (Setup.wValue.c[MSB] == DSC_DEVICE ||
			Setup.wValue.c[MSB] == DSC_CONFIG ||
			Setup.wValue.c[MSB] == DSC_STRING || Setup.wValue.c[MSB]
			== DSC_INTERFACE || Setup.wValue.c[MSB] == DSC_ENDPOINT)
	{
		if ((Setup.wLength.c[LSB] < UsbDataSize) && (Setup.wLength.c[MSB] == 0))
		{
			UsbDataSize = Setup.wLength.i;
		}
	}

	//����Ӧ��
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, rbSOPRDY);
		UsbEpStatus[0] = EP_TX;
		UsbDataSent = 0;
	}
}


/*****************************************************************************************************************
* ��	�ƣ�UsbGetConfiguration()
* ��	�ܣ�USB���û�ȡ����
* ��ڲ�������
* ���ڲ�������
* ˵	����USB��׼������
*****************************************************************************************************************/
void UsbGetConfiguration(void)  reentrant
{
	if ((Setup.bmRequestType != OUT_DEVICE) ||
			Setup.wValue.c[MSB] || Setup.wValue.c[LSB]||
			Setup.wIndex.c[MSB] || Setup.wIndex.c[LSB]||
			Setup.wLength.c[MSB] || (Setup.wLength.c[LSB] != 1))
	{
		UsbForceStall();
	}

	else
	{
		if (UsbState == DEV_CONFIGURED) 
		{
			UsbDataPtr = (BYTE*)&USB_ONES_PACKET;
			UsbDataSize = 1;
		}
		if (UsbState == DEV_ADDRESS)
		{
			UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
			UsbDataSize = 1;
		}
	}

	//�����ظ�
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, rbSOPRDY);
		UsbEpStatus[0] = EP_TX;
		UsbDataSent = 0;
	}
}


/*****************************************************************************************************************
* ��	�ƣ�UsbSetConfiguration()
* ��	�ܣ�USB�������ú���
* ��ڲ�������
* ���ڲ�������
* ˵	����USB��׼������
*****************************************************************************************************************/
void UsbSetConfiguration(void)  reentrant
{

	if ((UsbState == DEV_DEFAULT) ||
			(Setup.bmRequestType != IN_DEVICE) ||
			Setup.wIndex.c[MSB] || Setup.wIndex.c[LSB]||
			Setup.wLength.c[MSB] || Setup.wLength.c[LSB] || Setup.wValue.c[MSB]
			|| (Setup.wValue.c[LSB] > 1))
	{
		UsbForceStall();
	}

	else
	{
		if (Setup.wValue.c[LSB] > 0)
		{
			UsbState = DEV_CONFIGURED;
			UsbEpStatus[1] = EP_IDLE; 
			UsbEpStatus[2] = EP_IDLE;
			UsbWriteByte(INDEX, EP1_IN_IDX);		//���������õ��˵�1��
			UsbWriteByte(EINCSR2, rbInDIRSEL); 		//���˵�1����ΪIN

#ifndef	USB_EP_TRANS_MASS_STORAGE					//�������MassStorage��������������Ǳ����
			UsbHandleIn();
#endif

			UsbWriteByte(INDEX, EP0_IDX); 			//���������ûض˵�0��
		}
		else
		{
			UsbState = DEV_ADDRESS;
			UsbEpStatus[1] = EP_HALT;
			UsbEpStatus[2] = EP_HALT;
		}
	}

	//�����ظ�
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}


/*****************************************************************************************************************
* ��	�ƣ�UsbGetInterface()
* ��	�ܣ�USB�ӿڻ�ȡ����
* ��ڲ�������
* ���ڲ�������
* ˵	����USB��׼������
*****************************************************************************************************************/
void UsbGetInterface(void)  reentrant
{

	if ((UsbState != DEV_CONFIGURED) ||
			(Setup.bmRequestType != OUT_INTERFACE) ||
			Setup.wValue.c[MSB] ||Setup.wValue.c[LSB] ||
			Setup.wIndex.c[MSB] ||Setup.wIndex.c[LSB] ||
			Setup.wLength.c[MSB] ||(Setup.wLength.c[LSB] != 1))
	{
		UsbForceStall();
	}

	else
	{
		//�ظ�0
		UsbDataPtr = (BYTE*)&USB_ZERO_PACKET;
		UsbDataSize = 1;
	}

	//�����ظ�
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, rbSOPRDY);
		UsbEpStatus[0] = EP_TX;
		UsbDataSent = 0;
	}
}


/*****************************************************************************************************************
* ��	�ƣ�UsbSetInterface()
* ��	�ܣ�USB�ӿ����ú���
* ��ڲ�������
* ���ڲ�������
* ˵	����USB��׼������
*****************************************************************************************************************/
void UsbSetInterface(void)  reentrant
{
	if ((Setup.bmRequestType != IN_INTERFACE) ||
			Setup.wLength.c[MSB] ||Setup.wLength.c[LSB]||
			Setup.wValue.c[MSB]  ||Setup.wValue.c[LSB] || Setup.wIndex.c[MSB]
			||Setup.wIndex.c[LSB])
	{
		UsbForceStall();
	}

	//�����ظ�
	if (UsbEpStatus[0] != EP_STALL)
	{
		UsbWriteByte(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}


/*****************************************************************************************************************
* ��	�ƣ�OnUsbReset()
* ��	�ܣ�USB��λ������Ӧ����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void OnUsbReset(void) reentrant
{
	UsbState = DEV_DEFAULT; 				//���豸����ΪĬ��״̬

	UsbWriteByte(POWER, 0x01); 				//ʹ��USB���������ʹ�ܼ��

	//����ÿ������Ĭ��״̬
	UsbEpStatus[0] = EP_IDLE;
	UsbEpStatus[1] = EP_HALT;
	UsbEpStatus[2] = EP_HALT;
}

/*****************************************************************************************************************
* ��	�ƣ�OnUsbSuspend()
* ��	�ܣ�USB���߹���ʱ����Ӧ����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void OnUsbSuspend(void) reentrant
{
	//��ӹ�������ʱ���û��������
}

/*****************************************************************************************************************
* ��	�ƣ�OnUsbResume()
* ��	�ܣ�USB���߻ָ�ʱ�Ĵ�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void OnUsbResume(void) reentrant
{
	//��ӻָ������ǵ��û��������
}


/*****************************************************************************************************************
* ��	�ƣ�OnUsbSetup()
* ��	�ܣ�USB Setup������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void OnUsbSetup(void) reentrant
{
	BYTE control_reg, TempReg; 						//����˵���Ϣ����ʱ����

	UsbWriteByte(INDEX, EP0_IDX); 					//ѡ��˵�0
	UsbReadByte(E0CSR, control_reg); 				//��ȡ���ƼĴ���

	//���õ�ַ
	if(UsbEpStatus[0] == EP_ADDRESS)
	{
		UsbWriteByte(FADDR, Setup.wValue.c[LSB]);
		UsbEpStatus[0] = EP_IDLE;
	}

	//����쳣��־
	if (control_reg & rbSTSTL)
	{
		UsbWriteByte(E0CSR, 0);
		UsbEpStatus[0] = EP_IDLE;
		return;
	}

	//���������
	if (control_reg & rbSUEND)
	{
		UsbWriteByte(E0CSR, rbDATAEND);
		UsbWriteByte(E0CSR, rbSSUEND);
		UsbEpStatus[0] = EP_IDLE;
	}

	//������ڿ���ģʽ
	if(UsbEpStatus[0] == EP_IDLE)
	{
		//�ж϶˵�0�Ƿ������ݿɶ�ȡ������У�����
		if (control_reg & rbOPRDY)
		{
			UsbFifoRead(FIFO_EP0, (BYTE *)&Setup, 8);	   //��ȡSetup���ﴦ���

			//ת������һ���ֽڵ��������͵ı��룬ӦΪPC�Ϻ�KEIL��������Ĳ�һ��
			Setup.wValue.i = Setup.wValue .c[MSB] + 256*Setup.wValue.c[LSB];
			Setup.wIndex.i = Setup.wIndex .c[MSB] + 256*Setup.wIndex.c[LSB];
			Setup.wLength.i = Setup.wLength.c[MSB] + 256*Setup.wLength.c[LSB];

			//����USB�ı�׼��������
			switch (Setup.bRequest)
			{
				case GET_STATUS:
					UsbGetStatus();
					break;
	
				case CLEAR_FEATURE:
					UsbClearFeature();
					break;
	
				case SET_FEATURE:
					UsbSetFeature();
					break;
	
				case SET_ADDRESS:
					UsbSetAddress();
					break;
	
				case GET_DESCRIPTOR:
					UsbGetDescriptor();
					break;
	
				case GET_CONFIGURATION:
					UsbGetConfiguration();
					break;
	
				case SET_CONFIGURATION:
					UsbSetConfiguration();
					break;
	
				case GET_INTERFACE:
					UsbGetInterface();
					break;
	
				case SET_INTERFACE:
					UsbSetInterface();
					break;
					 
				default:
					UsbForceStall();		//���������Ч����ǿ��ֹͣ
					break;
			}
		}
	}

	//�鿴�˵�0�Ƿ���������Ҫ����
	if(UsbEpStatus[0] == EP_TX)
	{
		//ȷ������FIFO�����ǿյ�
		if((control_reg & rbINPRDY) == 0)
		{
			//��ȡ��ǰ��E0CSR�Ĵ���
			UsbReadByte(E0CSR, control_reg);

			//ȷ��ͨ��û�б���ֹ����û�н��յ��µ�����
			if((!(control_reg & rbSUEND)) || (!(control_reg & rbOPRDY)))
			{
				TempReg = rbINPRDY;              

				//������ݳ��ȱȶ˵����������󣬽����ݷֶ�
				if (UsbDataSize >= EP0_PACKET_SIZE)
				{
					UsbFifoWrite(FIFO_EP0, (BYTE*)UsbDataPtr, EP0_PACKET_SIZE);
					UsbDataPtr += EP0_PACKET_SIZE;
					UsbDataSize -= EP0_PACKET_SIZE;
					UsbDataSent += EP0_PACKET_SIZE;
				}
				else
				{
					UsbFifoWrite(FIFO_EP0, (BYTE *)UsbDataPtr, UsbDataSize);
					TempReg |= rbDATAEND; 			//�������ݽ�����־
					UsbEpStatus[0] = EP_IDLE; 		//���˵�0״̬����Ϊ����״̬
				}

				//�������
				if (UsbDataSent == Setup.wLength.i)
				{
					TempReg |= rbDATAEND; 			//�������ݽ�����־
					UsbEpStatus[0] = EP_IDLE; 		//���˵�0״̬����Ϊ����״̬
				}

				//����ƼĴ���д��ָ������
				UsbWriteByte(E0CSR, TempReg); // Write mask to E0CSR
			}
		}
	}
}

/*****************************************************************************************************************
* ��	�ƣ�UsbIsr()
* ��	�ܣ�USB�жϴ�����
* ��ڲ�������
* ���ڲ�������
* ˵	��������USB�ж���Ϣ
*****************************************************************************************************************/
void UsbIsr(void) interrupt 8
{
	BYTE bCommon, bIn, bOut;

	//�����ж�
	OSIntEnter();

	//��ȡ�ж�����USB�Ĵ���=====================================================================
	UsbReadByte(CMINT, bCommon); 				
	UsbReadByte(IN1INT, bIn); 					
	UsbReadByte(OUT1INT, bOut);

	//����======================================================================================
	
	//�ָ�����ʱ�Ĵ���
	if (bCommon & rbRSUINT)
	{
		OnUsbResume();
	}

 	//��λ����
	if (bCommon & rbRSTINT)
	{
		OnUsbReset();
	}

	//Setup���ﴦ��
	if (bIn & rbEP0)
	{
		OnUsbSetup();
	}

	//�������������ݵĴ������
	if (bIn & rbIN1)
	{
		UsbHandleIn();
	}

	//�������������ݵĴ������
	if (bOut & rbOUT2)
	{
		UsbHandleOut();
	}

	//��������ʱ�Ĵ���
	if (bCommon & rbSUSINT)
	{
		OnUsbSuspend();
	}

	//�˳��ж�
	OSIntExit();
}
