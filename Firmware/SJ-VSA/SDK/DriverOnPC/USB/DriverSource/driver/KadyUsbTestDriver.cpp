//������Դ�ļ�

#define VDW_MAIN
#include <vdw.h>
#include <kusb.h>
#include <kusbbusintf.h>
#include "function.h"
#include "KadyUsbTestDriver.h"
#include "KadyUsbTestDevice.h"

#pragma hdrstop("KadyUsbTest.pch")

//����һ���ڴ����
POOLTAG DefaultPoolTag('ydaK');

//����һ�����Զ���
KDebugOnlyTrace T("KadyUsbTest");

///////////////////////////////////////////////////////////////////////////////////////////////////
//��ʼ��ʼ�������
#pragma code_seg("INIT")

DECLARE_DRIVER_CLASS(KadyUsbTestDriver, NULL)

///////////////////////////////////////////////////////////////////////////////////////////////////
//����������ں�������������ʱϵͳ����ô˺���
NTSTATUS KadyUsbTestDriver::DriverEntry(PUNICODE_STRING RegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;

	m_Unit = 0;

	//��ȡע�����Ϣ���Գ�ʼ����Ա����
	LoadRegistryParameters(RegistryPath);

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//ע����ȡ����
void KadyUsbTestDriver::LoadRegistryParameters(PUNICODE_STRING RegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;

	//��Ӵ��룬��ʵ��ע���Ķ�ȡ
	UNICODE_STRING	regPath;
	KRegistryKey	RegKey;
    ULONG			length;

}

//������ʼ�������
#pragma code_seg()	

///////////////////////////////////////////////////////////////////////////////////////////////////
//����������Ӧ���豸����ʱ��ϵͳ����ô˺���
NTSTATUS KadyUsbTestDriver::AddDevice(PDEVICE_OBJECT Pdo)
{
	NTSTATUS status = STATUS_SUCCESS;

	//���������豸���󴴽��µĹ����豸����
	KadyUsbTestDevice* pDevice = new (
										NULL,                    // no name
										FILE_DEVICE_UNKNOWN,
										NULL,                    // no name
										0,
										DO_DIRECT_IO | DO_POWER_PAGABLE
										)
										KadyUsbTestDevice(Pdo, m_Unit);

	//�������ʧ�ܣ�����ϵͳ��Դ������Ϣ
	if (pDevice == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	else
	{
		//�����ɹ�����ȡ�豸�����״̬
		status = pDevice->ConstructorStatus();
		if (!NT_SUCCESS(status))
		{
			//ʧ�ܣ�ɾ������
			delete pDevice;
		}
		else
		{
			//�ɹ��������豸��Դ״̬
			m_Unit++;
			pDevice->ReportNewDevicePowerState(PowerDeviceD0);
		}
	}

	return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//�˳�������������ж��ʱϵͳ����ô˺���
VOID KadyUsbTestDriver::Unload(VOID)
{
	KDriver::Unload();
}
