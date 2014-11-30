//设备类源文件

#include <vdw.h>
#include <kusb.h>
#include <kusbbusintf.h>
#include "KadyUsbTestDriver.h"
#include "KadyUsbTestDevice.h"
#include "..\\intrface.h"

#pragma hdrstop("KadyUsbTest.pch")

//声明调试信息对象
extern KDebugOnlyTrace T;

///////////////////////////////////////////////////////////////////////////////////////////////////
//功能设备对象构造函数，使用了Pdo参数，并向KPnpDevice传入GUID(应用程序访问设备是需要的)
KadyUsbTestDevice::KadyUsbTestDevice(PDEVICE_OBJECT Pdo, ULONG Unit) :
	KadyUsbDvcName(NULL),
	KPnpDevice(Pdo, &GUID_DEVINTERFACE_KADYUSB340)
{
	//判断构造函数是否正确
	if (!NT_SUCCESS(m_ConstructorStatus))
	{
		ASSERT(FALSE);
		return;
	}

	//初始化下层接口
	m_Lower.Initialize(this, Pdo);

	//初始化USB接口(默认调用配置1的接口0)
	m_Interface.Initialize(
					m_Lower,		//USB设备
					0,				//接口0
					1,				//配置1
					0				//初始化接口
					); 

	//初始化USB端点
	EndPoint1In.Initialize(m_Lower, 0x81, 128);			//端点1，输入端点，128个字节
	EndPoint2Out.Initialize(m_Lower, 0x02, 256);		//端点2，输出端点，256个字节

#if (_WDM_ && (WDM_MAJORVERSION > 1 ||((WDM_MAJORVERSION == 1) && (WDM_MINORVERSION >= 0x20))))
	//初始化USB直接客户访问接口
	if (STATUS_SUCCESS == m_BusIntf.Initialize(m_Lower.TopOfStack()))
		m_fBusIntfAvailable = TRUE;
	else
		m_fBusIntfAvailable = FALSE;
#endif

	//将USB设备对象设置为下层设备
	SetLowerDevice(&m_Lower);

	//调用标准的即插即用初始化
	SetPnpPolicy();
	
	//调用标准的电源初始化
	SetPowerPolicy();

	//加载注册表信息
	LoadRegistryParameters();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//USB功能对象析构函数
KadyUsbTestDevice::~KadyUsbTestDevice()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//加载注册表信息
void KadyUsbTestDevice::LoadRegistryParameters()
{
    //读取驱动的注册表信息
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

		//读取键值
		status = RegKey.QueryValue(
                L"KadyUsbDvcName", 
				KadyUsbDvcName,
				length,
				PagedPool
				);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//设备接入时系统会调用此函数
NTSTATUS KadyUsbTestDevice::DefaultPnp(KIrp I)
{
	//这里使用默认处理
	I.ForceReuseOfCurrentStackLocationInCalldown();
	NTSTATUS status = m_Lower.PnpCall(this, I);

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//默认电源配置函数
NTSTATUS KadyUsbTestDevice::DefaultPower(KIrp I)
{
	//采用默认处理
	I.IndicatePowerIrpProcessed();
	I.CopyParametersDown();
	NTSTATUS status = m_Lower.PnpPowerCall(this, I);

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//系统控制函数
NTSTATUS KadyUsbTestDevice::SystemControl(KIrp I)
{
	NTSTATUS status = STATUS_SUCCESS;

	//采用默认处理
	I.ForceReuseOfCurrentStackLocationInCalldown();
	status = m_Lower.PnpCall(this, I);
	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//设备接入时会系统会调用此函数
NTSTATUS KadyUsbTestDevice::OnStartDevice(KIrp I)
{
	NTSTATUS status = STATUS_SUCCESS;

	I.Information() = 0;
	status = STATUS_UNSUCCESSFUL;

	AC_STATUS acStatus = AC_SUCCESS;

	acStatus = m_Lower.ActivateConfiguration(1);		//使用配置1

	switch (acStatus)
	{
		//配置成功
		case AC_SUCCESS:
			status = STATUS_SUCCESS;
			TestBusInterface();			//USB直接客户访问，测试总线接口
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
//停止设备时系统会调用此函数
NTSTATUS KadyUsbTestDevice::OnStopDevice(KIrp I)
{
	NTSTATUS status = STATUS_SUCCESS;

	//释放系统资源
	Invalidate();

	return STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//移除设备时系统会调用此函数
NTSTATUS KadyUsbTestDevice::OnRemoveDevice(KIrp I)
{
	T.Trace(TraceInfo, __FUNCTION__"++.  IRP %p\n", I);

	NTSTATUS status = STATUS_SUCCESS;

	//释放系统资源
	Invalidate();

	return STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//查询设备性能
NTSTATUS KadyUsbTestDevice::OnQueryCapabilities(KIrp I)
{
	//这里使用默认操作
	NTSTATUS status = STATUS_SUCCESS;

	I.CopyParametersDown();
	I.SetCompletionRoutine(LinkTo(OnQueryCapabilitiesComplete), this, TRUE, TRUE, TRUE);

	status = m_Lower.PnpCall(this, I);

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//查询性能结束
NTSTATUS KadyUsbTestDevice::OnQueryCapabilitiesComplete(KIrp I)
{
	//这里使用默认操作
	NTSTATUS status = I.Status();

	if (I->PendingReturned) I.MarkPending();

	if (NT_SUCCESS(status)) 
	{
		I.DeviceCapabilities()->SurpriseRemovalOK = TRUE;
	}

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//测试总线接口
void KadyUsbTestDevice::TestBusInterface()
{
#if (_WDM_ && (WDM_MAJORVERSION > 1 ||((WDM_MAJORVERSION == 1) && (WDM_MINORVERSION >= 0x20))))

	// 这里使用默认操作
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
//设备上电时，系统会调用此函数
NTSTATUS KadyUsbTestDevice::OnDevicePowerUp(KIrp I)
{
	//无操作
	NTSTATUS status = STATUS_SUCCESS;
	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//设备休眠时，系统会调用此函数
NTSTATUS KadyUsbTestDevice::OnDeviceSleep(KIrp I)
{
	//无操作
	NTSTATUS status = STATUS_SUCCESS;

	return status;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//应用程序创建设备对象时(应用程序调用CreateFile函数)，系统会调用此函数
NTSTATUS KadyUsbTestDevice::Create(KIrp I)
{
	//这里使用默认操作
	NTSTATUS status = STATUS_SUCCESS;
	I.Information() = 0;
	I.PnpComplete(this, status);
	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//应用程序释放设备对象时(应用程序调用CloseHandle函数)，系统会调用此函数
NTSTATUS KadyUsbTestDevice::Close(KIrp I)
{
	//这里使用默认操作
	NTSTATUS status = STATUS_SUCCESS;
	I.Information() = 0;
	I.PnpComplete(this, status);
	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//应用程序读设备数据时(应用程序调用ReadFile函数)，系统会调用此函数
NTSTATUS KadyUsbTestDevice::Read(KIrp I)
{
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	//声明变量，接收实际读取的字节数	
    ULONG dwBytesRead = 0;
	
	//URB
	PURB pUrb = NULL;
	
	//获取应用程序内存对象
	KMemory Mem(I.Mdl());

	//检查缓冲区长度
    ULONG dwTotalSize = I.ReadSize(CURRENT);				//应用程序要读取的字节数
	ULONG dwMaxSize = EndPoint1In.MaximumTransferSize();	//端点能传输的最大字节数
	if ( dwTotalSize > dwMaxSize )
	{
		//如果要读取的字节数超过端点的最大长度，则设置为最大长度
		ASSERT(dwMaxSize);
		dwTotalSize = dwMaxSize;
	}
	
	if(UsbdPipeTypeBulk == EndPoint2Out.Type())
	{
		pUrb = EndPoint1In.BuildBulkTransfer(
			Mem,		  //数据缓冲区
			dwTotalSize,  //要读取的字节数
			TRUE,         //输入
			NULL	      //下一个URB(无)
							);

	}
	else if(UsbdPipeTypeInterrupt == EndPoint2Out.Type())
	{
		pUrb = EndPoint1In.BuildInterruptTransfer(
			Mem,		  //数据缓冲区
			dwTotalSize,  //要读取的字节数
			TRUE,         //允许接收的字节数小于需求字节数
			NULL,	      //下一个URB(无)
			NULL,		  //当前USB(无，则创建新的)
			TRUE		  //读数据
													);
	}

	
	//判断创建URB是否成功
	if ( pUrb != NULL)
	{
		//设置为直接读取方式，允许接收字节数小于要求的字节数
		pUrb->UrbBulkOrInterruptTransfer.TransferFlags =
			(USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK);
		
		//提交URB
        status = EndPoint1In.SubmitUrb(pUrb, NULL, NULL,1000);
		
		//获取实际读取的字节数
        if ( NT_SUCCESS(status) ) 
        {
			dwBytesRead = pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;
		}
		
		delete pUrb;
	}
	
	//向应用程序返回实际读取的字节数
    I.Information() = dwBytesRead;

	//结束IRP操作
    return I.PnpComplete(this, status, IO_NO_INCREMENT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//应用程序向设备写数据时的操作
NTSTATUS KadyUsbTestDevice::Write(KIrp I)
{
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

	//声明变量，存放实际发送的字节数
    ULONG dwBytesSent = 0;
	
	//URB
	PURB pUrb = NULL;
	
	//获取应用程序内存对象
	KMemory Mem(I.Mdl());
	
	//检查缓冲区长度
	ULONG dwTotalSize = I.WriteSize(CURRENT);				//应用程序要写的数据长度
	ULONG dwMaxSize = EndPoint2Out.MaximumTransferSize();	//端点允许传输的最大数据长度
	if ( dwTotalSize > dwMaxSize )
	{
		//如果超过允许的最大数据长度，则传输最大允许数据长度个字节数
		ASSERT(dwMaxSize);
		dwTotalSize = dwMaxSize;
	}
	
	if(UsbdPipeTypeBulk == EndPoint2Out.Type())
	{
		pUrb = EndPoint2Out.BuildBulkTransfer(
			Mem,          //数据缓冲区
			dwTotalSize,  //需要传输的字节数
			FALSE,        //写数据
			NULL          //下一个URB(无)
							);
	}
	else if(UsbdPipeTypeInterrupt == EndPoint2Out.Type())
	{
		//创建URB
		pUrb = EndPoint2Out.BuildInterruptTransfer(
			Mem,		  //数据缓冲区
			dwTotalSize,  //需要传输的字节数
			TRUE,         //允许实际传输的字节数小于需要传输的字节数
			NULL,	      //下一个URB(无)
			NULL,		  //当前URB(无，创建新的)
			FALSE		  //写数据
													);
	}
	
    //判断URB创建是否成功
    if (pUrb != NULL) 
    {
		//提交URB
        status = EndPoint2Out.SubmitUrb(pUrb, NULL, NULL,1000);
		
		//获取实际写入的字节数
        if ( NT_SUCCESS(status) ) 
        {
            dwBytesSent = pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;
        }
		
		delete pUrb;
    }
	
	//向应用程序返回实际写入的字节数
    I.Information() = dwBytesSent;

	//完成IRP
    return I.PnpComplete(this, status, IO_NO_INCREMENT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//设备操作控制，对应应用程序的操作函数
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

	//完成操作
	if (status != STATUS_PENDING)
	{
		I.PnpComplete(this, status);
	}

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//清除信息
NTSTATUS KadyUsbTestDevice::CleanUp(KIrp I)
{
	NTSTATUS status = STATUS_SUCCESS;
	I.PnpComplete(this, status);
	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 释放资源
VOID KadyUsbTestDevice::Invalidate()
{
	NTSTATUS status = STATUS_SUCCESS;

	//解除配置
	status = m_Lower.DeActivateConfiguration();

	//释放下层设备资源
	m_Lower.ReleaseResources();

    // Free our registry value buffer
    if (KadyUsbDvcName != NULL)
    {
        delete KadyUsbDvcName;
        KadyUsbDvcName = NULL;
    }

}
