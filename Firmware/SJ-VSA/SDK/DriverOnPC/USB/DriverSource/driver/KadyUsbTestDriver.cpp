//驱动类源文件

#define VDW_MAIN
#include <vdw.h>
#include <kusb.h>
#include <kusbbusintf.h>
#include "function.h"
#include "KadyUsbTestDriver.h"
#include "KadyUsbTestDevice.h"

#pragma hdrstop("KadyUsbTest.pch")

//声明一个内存对象
POOLTAG DefaultPoolTag('ydaK');

//声明一个调试对象
KDebugOnlyTrace T("KadyUsbTest");

///////////////////////////////////////////////////////////////////////////////////////////////////
//开始初始化代码段
#pragma code_seg("INIT")

DECLARE_DRIVER_CLASS(KadyUsbTestDriver, NULL)

///////////////////////////////////////////////////////////////////////////////////////////////////
//驱动程序入口函数，驱动加载时系统会调用此函数
NTSTATUS KadyUsbTestDriver::DriverEntry(PUNICODE_STRING RegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;

	m_Unit = 0;

	//读取注册表信息，以初始化成员变量
	LoadRegistryParameters(RegistryPath);

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//注册表读取函数
void KadyUsbTestDriver::LoadRegistryParameters(PUNICODE_STRING RegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;

	//添加代码，以实现注册表的读取
	UNICODE_STRING	regPath;
	KRegistryKey	RegKey;
    ULONG			length;

}

//结束初始化代码段
#pragma code_seg()	

///////////////////////////////////////////////////////////////////////////////////////////////////
//当与驱动对应的设备接入时，系统会调用此函数
NTSTATUS KadyUsbTestDriver::AddDevice(PDEVICE_OBJECT Pdo)
{
	NTSTATUS status = STATUS_SUCCESS;

	//根据物理设备对象创建新的功能设备对象
	KadyUsbTestDevice* pDevice = new (
										NULL,                    // no name
										FILE_DEVICE_UNKNOWN,
										NULL,                    // no name
										0,
										DO_DIRECT_IO | DO_POWER_PAGABLE
										)
										KadyUsbTestDevice(Pdo, m_Unit);

	//如果创建失败，返回系统资源不足信息
	if (pDevice == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	else
	{
		//创建成功，获取设备对象的状态
		status = pDevice->ConstructorStatus();
		if (!NT_SUCCESS(status))
		{
			//失败，删除对象
			delete pDevice;
		}
		else
		{
			//成功，报告设备电源状态
			m_Unit++;
			pDevice->ReportNewDevicePowerState(PowerDeviceD0);
		}
	}

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//退出驱动程序，驱动卸载时系统会调用此函数
VOID KadyUsbTestDriver::Unload(VOID)
{
	KDriver::Unload();
}
