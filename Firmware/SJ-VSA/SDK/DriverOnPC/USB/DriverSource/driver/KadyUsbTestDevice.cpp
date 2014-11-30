//�豸��Դ�ļ�

#include <vdw.h>
#include <kusb.h>
#include <kusbbusintf.h>
#include "KadyUsbTestDriver.h"
#include "KadyUsbTestDevice.h"
#include "..\\intrface.h"

#pragma hdrstop("KadyUsbTest.pch")

//����������Ϣ����
extern KDebugOnlyTrace T;

///////////////////////////////////////////////////////////////////////////////////////////////////
//�����豸�����캯����ʹ����Pdo����������KPnpDevice����GUID(Ӧ�ó�������豸����Ҫ��)
KadyUsbTestDevice::KadyUsbTestDevice(PDEVICE_OBJECT Pdo, ULONG Unit) :
	KadyUsbDvcName(NULL),
	KPnpDevice(Pdo, &GUID_DEVINTERFACE_KADYUSB340)
{
	//�жϹ��캯���Ƿ���ȷ
	if (!NT_SUCCESS(m_ConstructorStatus))
	{
		ASSERT(FALSE);
		return;
	}

	//��ʼ���²�ӿ�
	m_Lower.Initialize(this, Pdo);

	//��ʼ��USB�ӿ�(Ĭ�ϵ�������1�Ľӿ�0)
	m_Interface.Initialize(
					m_Lower,		//USB�豸
					0,				//�ӿ�0
					1,				//����1
					0				//��ʼ���ӿ�
					); 

	//��ʼ��USB�˵�
	EndPoint1In.Initialize(m_Lower, 0x81, 128);			//�˵�1������˵㣬128���ֽ�
	EndPoint2Out.Initialize(m_Lower, 0x02, 256);		//�˵�2������˵㣬256���ֽ�

#if (_WDM_ && (WDM_MAJORVERSION > 1 ||((WDM_MAJORVERSION == 1) && (WDM_MINORVERSION >= 0x20))))
	//��ʼ��USBֱ�ӿͻ����ʽӿ�
	if (STATUS_SUCCESS == m_BusIntf.Initialize(m_Lower.TopOfStack()))
		m_fBusIntfAvailable = TRUE;
	else
		m_fBusIntfAvailable = FALSE;
#endif

	//��USB�豸��������Ϊ�²��豸
	SetLowerDevice(&m_Lower);

	//���ñ�׼�ļ��弴�ó�ʼ��
	SetPnpPolicy();
	
	//���ñ�׼�ĵ�Դ��ʼ��
	SetPowerPolicy();

	//����ע�����Ϣ
	LoadRegistryParameters();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//USB���ܶ�����������
KadyUsbTestDevice::~KadyUsbTestDevice()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//����ע�����Ϣ
void KadyUsbTestDevice::LoadRegistryParameters()
{
    //��ȡ������ע�����Ϣ
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING	regPath;
	KRegistryKey	RegKey;
    ULONG			length;


	status = RegKey.Initialize(
				m_Lower.DeviceObject(),
                PLUGPLAY_REGKEY_DRIVER,
                KEY_ALL_ACCESS
				);

	if (NT_SUCCESS(status))
	{
		length = 0;

		//��ȡ��ֵ
		status = RegKey.QueryValue(
                L"KadyUsbDvcName", 
				KadyUsbDvcName,
				length,
				PagedPool
				);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//�豸����ʱϵͳ����ô˺���
NTSTATUS KadyUsbTestDevice::DefaultPnp(KIrp I)
{
	//����ʹ��Ĭ�ϴ���
	I.ForceReuseOfCurrentStackLocationInCalldown();
	NTSTATUS status = m_Lower.PnpCall(this, I);

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//Ĭ�ϵ�Դ���ú���
NTSTATUS KadyUsbTestDevice::DefaultPower(KIrp I)
{
	//����Ĭ�ϴ���
	I.IndicatePowerIrpProcessed();
	I.CopyParametersDown();
	NTSTATUS status = m_Lower.PnpPowerCall(this, I);

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//ϵͳ���ƺ���
NTSTATUS KadyUsbTestDevice::SystemControl(KIrp I)
{
	NTSTATUS status = STATUS_SUCCESS;

	//����Ĭ�ϴ���
	I.ForceReuseOfCurrentStackLocationInCalldown();
	status = m_Lower.PnpCall(this, I);
	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//�豸����ʱ��ϵͳ����ô˺���
NTSTATUS KadyUsbTestDevice::OnStartDevice(KIrp I)
{
	NTSTATUS status = STATUS_SUCCESS;

	I.Information() = 0;
	status = STATUS_UNSUCCESSFUL;

	AC_STATUS acStatus = AC_SUCCESS;

	acStatus = m_Lower.ActivateConfiguration(1);		//ʹ������1

	switch (acStatus)
	{
		//���óɹ�
		case AC_SUCCESS:
			status = STATUS_SUCCESS;
			TestBusInterface();			//USBֱ�ӿͻ����ʣ��������߽ӿ�
			break;

		case AC_COULD_NOT_LOCATE_INTERFACE:
			break;

		case AC_COULD_NOT_PRECONFIGURE_INTERFACE:
			break;

		case AC_CONFIGURATION_REQUEST_FAILED:
			break;

		case AC_FAILED_TO_INITIALIZE_INTERFACE_OBJECT:
			break;

		case AC_FAILED_TO_GET_DESCRIPTOR:
			break;

		case AC_FAILED_TO_OPEN_PIPE_OBJECT:
			status = STATUS_SUCCESS;
			break;

		default:
			break;
	}

	// TODO:	Add USB device-specific code to start the hardware.

	T.Trace(TraceInfo, __FUNCTION__"--.  IRP %p, STATUS %x\n", I, status);

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//ֹͣ�豸ʱϵͳ����ô˺���
NTSTATUS KadyUsbTestDevice::OnStopDevice(KIrp I)
{
	NTSTATUS status = STATUS_SUCCESS;

	//�ͷ�ϵͳ��Դ
	Invalidate();

	return STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//�Ƴ��豸ʱϵͳ����ô˺���
NTSTATUS KadyUsbTestDevice::OnRemoveDevice(KIrp I)
{
	T.Trace(TraceInfo, __FUNCTION__"++.  IRP %p\n", I);

	NTSTATUS status = STATUS_SUCCESS;

	//�ͷ�ϵͳ��Դ
	Invalidate();

	return STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//��ѯ�豸����
NTSTATUS KadyUsbTestDevice::OnQueryCapabilities(KIrp I)
{
	//����ʹ��Ĭ�ϲ���
	NTSTATUS status = STATUS_SUCCESS;

	I.CopyParametersDown();
	I.SetCompletionRoutine(LinkTo(OnQueryCapabilitiesComplete), this, TRUE, TRUE, TRUE);

	status = m_Lower.PnpCall(this, I);

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//��ѯ���ܽ���
NTSTATUS KadyUsbTestDevice::OnQueryCapabilitiesComplete(KIrp I)
{
	//����ʹ��Ĭ�ϲ���
	NTSTATUS status = I.Status();

	if (I->PendingReturned) I.MarkPending();

	if (NT_SUCCESS(status)) 
	{
		I.DeviceCapabilities()->SurpriseRemovalOK = TRUE;
	}

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//�������߽ӿ�
void KadyUsbTestDevice::TestBusInterface()
{
#if (_WDM_ && (WDM_MAJORVERSION > 1 ||((WDM_MAJORVERSION == 1) && (WDM_MINORVERSION >= 0x20))))

	// ����ʹ��Ĭ�ϲ���
	if (m_fBusIntfAvailable) 
	{
		USBD_VERSION_INFORMATION UsbVerInfo;
		RtlZeroMemory(&UsbVerInfo, sizeof(USBD_VERSION_INFORMATION));
		ULONG HcdCapabilities = 0;

		m_BusIntf.GetUSBDIVersion(&UsbVerInfo, &HcdCapabilities);


		ULONG TotalBW, ConsumedBW;
		NTSTATUS Status = m_BusIntf.GetBandwidth(&TotalBW,&ConsumedBW);

		if (STATUS_SUCCESS == Status) 
		{
		}

		PWSTR HcName = NULL;
		Status = m_BusIntf.GetControllerName(HcName);

		if (STATUS_SUCCESS == Status && HcName) 
		{
			delete HcName;
		}

		ULONG CurrentFrame;
		m_BusIntf.QueryBusTime(&CurrentFrame);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//�豸�ϵ�ʱ��ϵͳ����ô˺���
NTSTATUS KadyUsbTestDevice::OnDevicePowerUp(KIrp I)
{
	//�޲���
	NTSTATUS status = STATUS_SUCCESS;
	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//�豸����ʱ��ϵͳ����ô˺���
NTSTATUS KadyUsbTestDevice::OnDeviceSleep(KIrp I)
{
	//�޲���
	NTSTATUS status = STATUS_SUCCESS;

	return status;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//Ӧ�ó��򴴽��豸����ʱ(Ӧ�ó������CreateFile����)��ϵͳ����ô˺���
NTSTATUS KadyUsbTestDevice::Create(KIrp I)
{
	//����ʹ��Ĭ�ϲ���
	NTSTATUS status = STATUS_SUCCESS;
	I.Information() = 0;
	I.PnpComplete(this, status);
	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//Ӧ�ó����ͷ��豸����ʱ(Ӧ�ó������CloseHandle����)��ϵͳ����ô˺���
NTSTATUS KadyUsbTestDevice::Close(KIrp I)
{
	//����ʹ��Ĭ�ϲ���
	NTSTATUS status = STATUS_SUCCESS;
	I.Information() = 0;
	I.PnpComplete(this, status);
	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//Ӧ�ó�����豸����ʱ(Ӧ�ó������ReadFile����)��ϵͳ����ô˺���
NTSTATUS KadyUsbTestDevice::Read(KIrp I)
{
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	//��������������ʵ�ʶ�ȡ���ֽ���	
    ULONG dwBytesRead = 0;
	
	//URB
	PURB pUrb = NULL;
	
	//��ȡӦ�ó����ڴ����
	KMemory Mem(I.Mdl());

	//��黺��������
    ULONG dwTotalSize = I.ReadSize(CURRENT);				//Ӧ�ó���Ҫ��ȡ���ֽ���
	ULONG dwMaxSize = EndPoint1In.MaximumTransferSize();	//�˵��ܴ��������ֽ���
	if ( dwTotalSize > dwMaxSize )
	{
		//���Ҫ��ȡ���ֽ��������˵����󳤶ȣ�������Ϊ��󳤶�
		ASSERT(dwMaxSize);
		dwTotalSize = dwMaxSize;
	}
	
	if(UsbdPipeTypeBulk == EndPoint2Out.Type())
	{
		pUrb = EndPoint1In.BuildBulkTransfer(
			Mem,		  //���ݻ�����
			dwTotalSize,  //Ҫ��ȡ���ֽ���
			TRUE,         //����
			NULL	      //��һ��URB(��)
							);

	}
	else if(UsbdPipeTypeInterrupt == EndPoint2Out.Type())
	{
		pUrb = EndPoint1In.BuildInterruptTransfer(
			Mem,		  //���ݻ�����
			dwTotalSize,  //Ҫ��ȡ���ֽ���
			TRUE,         //������յ��ֽ���С�������ֽ���
			NULL,	      //��һ��URB(��)
			NULL,		  //��ǰUSB(�ޣ��򴴽��µ�)
			TRUE		  //������
													);
	}

	
	//�жϴ���URB�Ƿ�ɹ�
	if ( pUrb != NULL)
	{
		//����Ϊֱ�Ӷ�ȡ��ʽ����������ֽ���С��Ҫ����ֽ���
		pUrb->UrbBulkOrInterruptTransfer.TransferFlags =
			(USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK);
		
		//�ύURB
        status = EndPoint1In.SubmitUrb(pUrb, NULL, NULL,1000);
		
		//��ȡʵ�ʶ�ȡ���ֽ���
        if ( NT_SUCCESS(status) ) 
        {
			dwBytesRead = pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;
		}
		
		delete pUrb;
	}
	
	//��Ӧ�ó��򷵻�ʵ�ʶ�ȡ���ֽ���
    I.Information() = dwBytesRead;

	//����IRP����
    return I.PnpComplete(this, status, IO_NO_INCREMENT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//Ӧ�ó������豸д����ʱ�Ĳ���
NTSTATUS KadyUsbTestDevice::Write(KIrp I)
{
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	//�������������ʵ�ʷ��͵��ֽ���
    ULONG dwBytesSent = 0;
	
	//URB
	PURB pUrb = NULL;
	
	//��ȡӦ�ó����ڴ����
	KMemory Mem(I.Mdl());
	
	//��黺��������
	ULONG dwTotalSize = I.WriteSize(CURRENT);				//Ӧ�ó���Ҫд�����ݳ���
	ULONG dwMaxSize = EndPoint2Out.MaximumTransferSize();	//�˵��������������ݳ���
	if ( dwTotalSize > dwMaxSize )
	{
		//������������������ݳ��ȣ���������������ݳ��ȸ��ֽ���
		ASSERT(dwMaxSize);
		dwTotalSize = dwMaxSize;
	}
	
	if(UsbdPipeTypeBulk == EndPoint2Out.Type())
	{
		pUrb = EndPoint2Out.BuildBulkTransfer(
			Mem,          //���ݻ�����
			dwTotalSize,  //��Ҫ������ֽ���
			FALSE,        //д����
			NULL          //��һ��URB(��)
							);
	}
	else if(UsbdPipeTypeInterrupt == EndPoint2Out.Type())
	{
		//����URB
		pUrb = EndPoint2Out.BuildInterruptTransfer(
			Mem,		  //���ݻ�����
			dwTotalSize,  //��Ҫ������ֽ���
			TRUE,         //����ʵ�ʴ�����ֽ���С����Ҫ������ֽ���
			NULL,	      //��һ��URB(��)
			NULL,		  //��ǰURB(�ޣ������µ�)
			FALSE		  //д����
													);
	}
	
    //�ж�URB�����Ƿ�ɹ�
    if (pUrb != NULL) 
    {
		//�ύURB
        status = EndPoint2Out.SubmitUrb(pUrb, NULL, NULL,1000);
		
		//��ȡʵ��д����ֽ���
        if ( NT_SUCCESS(status) ) 
        {
            dwBytesSent = pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;
        }
		
		delete pUrb;
    }
	
	//��Ӧ�ó��򷵻�ʵ��д����ֽ���
    I.Information() = dwBytesSent;

	//���IRP
    return I.PnpComplete(this, status, IO_NO_INCREMENT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//�豸�������ƣ���ӦӦ�ó���Ĳ�������
NTSTATUS KadyUsbTestDevice::DeviceControl(KIrp I)
{
	NTSTATUS status = STATUS_SUCCESS;

	switch (I.IoctlCode())
	{
    case 0:

	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	//��ɲ���
	if (status != STATUS_PENDING)
	{
		I.PnpComplete(this, status);
	}

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//�����Ϣ
NTSTATUS KadyUsbTestDevice::CleanUp(KIrp I)
{
	NTSTATUS status = STATUS_SUCCESS;
	I.PnpComplete(this, status);
	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// �ͷ���Դ
VOID KadyUsbTestDevice::Invalidate()
{
	NTSTATUS status = STATUS_SUCCESS;

	//�������
	status = m_Lower.DeActivateConfiguration();

	//�ͷ��²��豸��Դ
	m_Lower.ReleaseResources();

    // Free our registry value buffer
    if (KadyUsbDvcName != NULL)
    {
        delete KadyUsbDvcName;
        KadyUsbDvcName = NULL;
    }

}
