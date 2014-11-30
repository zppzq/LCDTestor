/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: SdScsi.c
**��   ��   ��: ��п�(kady1984@163.com)
**�� �� ��  ��: 2008��04��20��
**����޸�����: 2008��04��20��
**��        ��: SCSIЭ��ʵ�ֺ���
*****************************************************************************************************************/
#include <string.h>
#include "SdSet.h"
#include "SdSectServer.h"
#include "SdScsi.h"
#include "..\Communication\UsbInclude.h"

//=========================================bulk-only������=============================================
//
//bulk-only������������ȶ�U�̵�Out�˵㷢��Out���Ʒ����������31���ֽڵ�cbwͨ�����ݷ��������Out�˵㣬���
//��������Ҫ�����������Ļ�����In�˵㷢��In���ƺ��ٽ�������Ҫ�����ݣ���ɺ�����In�˵㷢��In���ƣ�����CSW
//
//=======================================================================================================



//CBW������ͨ��Bulk-Out�˵����豸���͵�����������CBW��ʹ�÷���λ�����ݴ��䳤����ָ����
//���Ĵ��䣬CBW������ʼ�ڰ��߽磬���ұ�����31�ֽڵĶ̰������������̵����ݰ���CSW������
//��ʼ��һ���µİ��߽磬���е�CBW�����밴���ֽ���ǰ�Ĵ�����
typedef struct
{
	DWORD dCBWSignature; 				//��������ʶ��CBW����ǣ���������һ��CBW����������ֵΪ0x43425355
	DWORD dCBWTag;						//������ǣ����豸������Ӧ��CSW��ʱ������ʹ����״̬������ֵ���ֵ��ͬ
	DWORD dCBWDataTransferLength;		//���ݴ��䳤�ȣ�ָ������ִ���ڼ���Bulk�˵��ϴ����ݵ��ֽڳ���
	BYTE bmCBWFlags;					//�������죺����λ�涨��Bulk�˵����ݴ���ķ�������λԤ��
	BYTE bCBWLUN;						//�߼���Ԫ�ţ�ָ������鱻���͵����߼���Ԫ�ţ�����豸��֧�ֶ���߼���Ԫ�ţ������������������Ϊ0
	BYTE bCBWCBLength;					//CBWCB���ȣ�������CBWCB����Ч���ȣ��Ϸ�ֵΪ1-16
	BYTE CBWCB[16];						//CBWCB�����豸ִ�е�������豸����
} CBW;


//CSW����������������CBW����������ִ��״̬���豸�յ�CBW�����������ͨ��Bulk-In�˵㷢��һ��CSW��
typedef struct
{
	DWORD dCSWSignature; 				//����״̬����ʶ��CSW���ı�ǣ���������һ��CSW����������ֵΪ53425355H
	DWORD dCSWTag;						//����״̬��ǣ������ֵ��CBW�������������ͬ
	DWORD dCSWDataResidue;				//���ݲ��ࣺʵ�����ݴ�������CBW���й涨�����ݴ��䳤�ȵĲ�ֵ
	BYTE bCSWStatus;					//����ִ��״̬����������ɹ���ʧ����Ϣ(0 - �ɹ�����0 - ʧ��)
} CSW;




//============================================Mass Storage ����=============================================
//
//USB�豸��Ϊ5���࣬����ʾ����ͨ���豸����Ƶ�豸���˻�����ͺ����洢��ͨ�����õ�U�̡��ƶ�Ӳ�̾����ں����洢�ࡣ
//�����洢��Ĺ淶�а���4���������ӹ淶����CBI���䡢Bulk-Only���䡢ATA����顢UFI����淶��ǰ����Э�鶨������
//��\����\״̬��USB�����ϵĴ��䷽����Bulk-Only����Э�����ʹ��Bulk�˵㴫������/����/״̬��CBI����Э����ʹ��
//Control/bulk/interrupt�������͵Ķ˵��������/����/״̬�Ĵ��͡�������Э�鶨���˴洢���ʵĲ������ATAЭ����
//��Ӳ�̣�UFIЭ�������USB�ƶ��洢��U�̶�д���������ѭBulk-Only����Э���UFI����淶��UFI�����淶�����USB
//�ƶ��洢���ƶ��ģ����ܹ�������19��12�ֽڳ��ȵĲ������
//
//============================================================================================================




//=================================================UFI�����붨��==============================================
//
//UFI��SCSI���Ӽ�
//
//============================================================================================================
#define SCSI_TEST_UNIT_READY 				0x00					//�����豸�����Ƿ���Ready״̬
#define SCSI_REQUEST_SENSE 					0x03					//�����豸����������ִ�н������״̬����
#define SCSI_FORMAT_UNIT					0x04	  				//��ʽ���洢��Ԫ
#define SCSI_SEND_DIAGNOSTIC				0x10
#define SCSI_INQUIRY						0x12					//��ȡ������Ϣ
#define SCSI_MODE_SELECT_6					0x15
#define SCSI_MODE_SENSE_6					0x1A
#define SCSI_START_STOP_UNIT				0x1B					//load/unload
#define SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL 	0x1E					//д����
#define SCSI_READ_CAPACITY_10				0x25					//Ҫ���豸���ص�ǰ����
#define SCSI_READ_CAPACITY_16				0x9E
#define SCSI_READ_6							0x08
#define SCSI_READ_10						0x28					//Host���洢�����еĶ���������
#define SCSI_READ_16						0x88
#define SCSI_WRITE_10						0x2A					//�����������д����������
#define SCSI_VERIFY_10						0x2F					//�ڴ洢����֤����
#define SCSI_READ_FORMAT_CAPACITIES 		0x23					//��ѯ��ǰ���������ÿռ�

//SCSI��׼��������=============================================================================================
#define DIRECTION_IN	0x80
#define DIRECTION_OUT	0x00
#define CBW_SIGNATURE 				0x55534243
#define CSW_SIGNATURE 				0x55534253

//SCSIЭ�����̲��趨��=========================================================================================
#define MSD_READY 					0x00
#define MSD_COMMAND_TRANSPORT 		0x01
#define MSD_DATA_IN					0x02
#define MSD_DATA_OUT				0x03
#define MSD_STATUS_TRANSPORT		0x04
#define MSD_DATA					0x05
#define MSD_DO_RESET				0xFF

//SCSI״̬����
#define SCSI_PASSED 		0
#define SCSI_FAILED 		1
#define SCSI_PHASE_ERROR 	2

//SCSI��Ϣ�ֶζ���=================================================================================================
//ģʽ��Ϣ
code const BYTE Scsi_Mode_Sense_6[4]= {0x03, 0, 0, 0}; 					// No mode sense parameter
//��׼���豸��Ϣ
code const BYTE Scsi_Standard_Inquiry_Data[28]=
{
	0x00, 																// Peripheral qualifier & peripheral device type
	0x80, 																// Removable medium
	0x05, 																// Version of the standard (2=obsolete, 5=SPC-3)
	0x02, 																// No NormACA, No HiSup, response data format=2
	0x1F, 																// No extra parameters
	0x00, 																// No flags
	0x80, 																// 0x80 => BQue => Basic Task Management supported
	0x00, 																// No flags
	'K','a','d','y',' ',' ',' ',' ',
	'M','a','s','s',' ',
	'S','t','o','r','a','g','e' 										// Requested by Dekimo via www.t10.org
};
//������Ϣ
BYTE xdata Scsi_Read_Capacity_10[8]=
{
	0x00,0x00,0xF4,0x5F, 											// Last logical block address
	0x00,0x00,msb(SectBlockSize()),lsb(SectBlockSize()) 			// Block length
};

//�ڲ�ȫ�ֱ�������================================================================================================
BYTE xdata MsdState = MSD_READY;
BYTE xdata ScsiStatus;
DWORD xdata ScsiResidue;
CBW xdata cbw;
CSW xdata csw;


//��������========================================================================================================

/*****************************************************************************************************************
* ��	�ƣ�ScsiSend()
* ��	�ܣ�����SISC����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
static void ScsiSend(BYTE* ptr, unsigned count) reentrant
{
	if (ScsiResidue < count)
	{
		ScsiStatus = SCSI_PHASE_ERROR;
		return;
	}
	ScsiResidue -= count;
	Usb0DeviceSend(ptr, count, 0);
}

/*****************************************************************************************************************
* ��	�ƣ�ScsiInquiry()
* ��	�ܣ���ѯUSB�豸��Ϣ
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void ScsiInquiry() reentrant
{
	ScsiStatus=SCSI_PASSED;
	ScsiSend(Scsi_Standard_Inquiry_Data, sizeof(Scsi_Standard_Inquiry_Data));
}

/*****************************************************************************************************************
* ��	�ƣ�ScsiReadCapacity10()
* ��	�ܣ���ѯSD��������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void ScsiReadCapacity10() reentrant
{
	unsigned int s;
	unsigned long size = SdSectors();
	size -= 1;
	s = ((size&0xFFFF0000) >> 16);
	Scsi_Read_Capacity_10[0]=msb((s));
	Scsi_Read_Capacity_10[1]=lsb((s));
	Scsi_Read_Capacity_10[2]=msb(size);
	Scsi_Read_Capacity_10[3]=lsb(size);

	ScsiStatus = SCSI_PASSED;
	ScsiSend(Scsi_Read_Capacity_10, sizeof(Scsi_Read_Capacity_10));
}

/*****************************************************************************************************************
* ��	�ƣ�ScsiRead10()
* ��	�ܣ�SCSI������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void ScsiRead10() reentrant
{
	int i, j;
	DWORD xdata d_len = ntohl(cbw.dCBWDataTransferLength);
	DWORD xdata d_LBA = cbw.CBWCB[2];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[3];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[4];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[5];

	for (i=0; i<(d_len+SectBlockSize()-1)/SectBlockSize(); i++)
	{
		SectRead(d_LBA+i);
		for (j = 0; j < (SectBlockSize()+EP1_PACKET_SIZE-1)/EP1_PACKET_SIZE; j++)
		{
			Usb0DeviceSend(Scratch + j * EP1_PACKET_SIZE, EP1_PACKET_SIZE, 0);
			ScsiResidue -= EP1_PACKET_SIZE;
		}
	}

	ScsiStatus = SCSI_PASSED;
}

/*****************************************************************************************************************
* ��	�ƣ�ScsiWrite10()
* ��	�ܣ�д���ݵ�SD��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void ScsiWrite10() reentrant
{
	int i, j;
	DWORD xdata d_len = ntohl(cbw.dCBWDataTransferLength);
	DWORD xdata d_LBA = cbw.CBWCB[2];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[3];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[4];
	d_LBA <<= 8;
	d_LBA += cbw.CBWCB[5];

	for (i=0; i<(d_len+SectBlockSize()-1)/SectBlockSize(); i++)
	{
		for (j=0; j<(SectBlockSize()+EP2_PACKET_SIZE-1)/EP2_PACKET_SIZE; j++)
		{
			//kady begin
			Usb0DeviceRecv(Scratch+j*EP2_PACKET_SIZE, EP2_PACKET_SIZE, 0);
			//kady end
		}
		SectWrite(d_LBA+i);
		ScsiResidue-=SectBlockSize();
	}
	ScsiStatus=SCSI_PASSED;
}

/*****************************************************************************************************************
* ��	�ƣ�ScsiModeSense6()
* ��	�ܣ�SCSIģʽ��ѯ
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void ScsiModeSense6()	reentrant
{
	ScsiStatus = SCSI_PASSED;
	ScsiSend(Scsi_Mode_Sense_6, sizeof(Scsi_Mode_Sense_6));
}

/*****************************************************************************************************************
* ��	�ƣ�ScsiRx()
* ��	�ܣ�SCSI���ݽ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void ScsiRx() reentrant
{
	int xdata i;

	ScsiStatus = SCSI_FAILED;
	ScsiResidue = ntohl(cbw.dCBWDataTransferLength);

	if (cbw.bCBWCBLength == 0) return; 					//������ݳ�����Ч��

	//����SCSI����
	switch (cbw.CBWCB[0])
	{
		case SCSI_TEST_UNIT_READY:
			ScsiStatus = SCSI_PASSED;
			break;

		case SCSI_INQUIRY:
			ScsiInquiry();
			break;

		case SCSI_MODE_SENSE_6:
			ScsiModeSense6();
			break;

		case SCSI_READ_CAPACITY_10:
			ScsiReadCapacity10();
			break;

		case SCSI_READ_10:
			ScsiRead10();
			break;

		case SCSI_WRITE_10:
			ScsiWrite10();
			break;

		case SCSI_VERIFY_10:
			ScsiResidue = 0;
			ScsiStatus = SCSI_PASSED;
			break;

		case SCSI_START_STOP_UNIT:
			ScsiStatus = SCSI_PASSED;
			break;

		case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
			ScsiStatus = SCSI_PASSED;
			break;

		default:
			break;
	}

	if (ScsiResidue && (ScsiResidue == ntohl(cbw.dCBWDataTransferLength)))
	{
		//���㷢�ͻ�����
		for (i=0; i<EP1_PACKET_SIZE; i++)
		{
			UsbInBuffer[i]=0;
		}

		//���������Ϳ�������
		while(ScsiResidue)
		{
			Usb0DeviceSend(UsbInBuffer, (EP1_PACKET_SIZE>ScsiResidue) ? ScsiResidue : EP1_PACKET_SIZE, 0);
			ScsiResidue -= ((EP1_PACKET_SIZE>ScsiResidue) ? ScsiResidue : EP1_PACKET_SIZE);
		}
	}
}

/*****************************************************************************************************************
* ��	�ƣ�SdScsiServerStep()
* ��	�ܣ�SCSIЭ���������
* ��ڲ�������
* ���ڲ�������
* ˵	�������û�������Ӧ��ѭ�����ô˲���
*****************************************************************************************************************/
void SdScsiServerStep() reentrant
{
	unsigned nRecvCount;

	switch (MsdState)
	{
		case MSD_READY:
			nRecvCount = Usb0DeviceRecv(UsbOutBuffer, 1000, 0);

			//����ǲ���CBW��
			if (nRecvCount != sizeof(CBW))
			{
				return;
			}
	
			//�����
			memcpy(&cbw, UsbOutBuffer, nRecvCount);
	
			//����־λ����Ч��
			if ((cbw.dCBWSignature!=CBW_SIGNATURE) 
					|| ((cbw.bmCBWFlags != DIRECTION_IN && cbw.bmCBWFlags != DIRECTION_OUT)
					|| (cbw.bCBWLUN&0xF0) 
					|| (cbw.bCBWCBLength>16))
					|| (cbw.bCBWLUN!=0x00))
			{
				return;
			}
			
			//�л������ݶ�ȡ״̬	
			MsdState = MSD_DATA;
			break;
	
		case MSD_DATA:
			ScsiRx();										//��SCSIЭ���ȡ����
			MsdState = MSD_STATUS_TRANSPORT;  				//�л������ݷ���״̬
			break;
	
		case MSD_STATUS_TRANSPORT:
			//�ظ�һ��CSW�ṹ:
			csw.dCSWSignature = CSW_SIGNATURE;
			csw.dCSWTag = cbw.dCBWTag;
			csw.bCSWStatus = ScsiStatus;
			csw.dCSWDataResidue = ntohl(ScsiResidue);
	
			//��������
			Usb0DeviceSend((BYTE*)&csw, sizeof(CSW), 0);
	
			MsdState = MSD_READY;							//�л�������״̬
			break;
	
		case MSD_DO_RESET:
			break;
	
		default:
			MsdState = MSD_READY;							//Ĭ�ϵ�����״̬
			break;
	}
}



