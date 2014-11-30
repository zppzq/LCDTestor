#include "StdAfx.h"
#include "KdUsb.h"

//ͷ�ļ�
#include "usb100.h"
#include <initguid.h>
#include <setupapi.h>
#pragma comment (lib, "setupapi.lib")

//�����շ���󳤶ȶ���========================================================================
//ע�⣺��������������һ��Ҫ���豸�ϱ���һ�£����߸�С����������
#define  EP1_PACKET_SIZE         	0x0040      //�˵�1���������ȣ����ڷ������ݵ�����(���128�ֽ�)
#define  EP2_PACKET_SIZE         	0x0040      //�˵�2���������ȣ����ڴ�������������(���256�ֽ�)


//�豸��ȫ��ID����============================================================================
DEFINE_GUID(GUID_DEVINTERFACE_KADYUSB340,
			0xF4BDDAC5, 0xEE5B, 0x449C, 0x91, 0x47, 0xCC, 0x00, 0x42, 0x47, 0xFB, 0x8A);

CKdUsb::CKdUsb(void)
{
	m_nCurDevice = -1;
	m_hDevice = NULL;
}

CKdUsb::~CKdUsb(void)
{
	Close();	
}

//ö���豸
BOOL CKdUsb::EnumDevices()
{
	//��������
	LONG i = 0;
	DWORD nBufferSize;
	SP_DEVICE_INTERFACE_DETAIL_DATA* pDeviceDetail = NULL;

	//�����������
	m_arrDeviceName.RemoveAll();

	//Ѱ��������ƥ����豸
	HDEVINFO hDeviceInfo = SetupDiGetClassDevs(
												(LPGUID)&GUID_DEVINTERFACE_KADYUSB340,
												NULL,
												NULL,
												DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
												);
	if(hDeviceInfo == INVALID_HANDLE_VALUE) return FALSE;

	//�ӿ�����
	SP_DEVICE_INTERFACE_DATA InterfaceData;
	InterfaceData.cbSize = sizeof(InterfaceData);

	//ö���豸
	for(i = 0; SetupDiEnumDeviceInterfaces(hDeviceInfo,NULL,(LPGUID)&GUID_DEVINTERFACE_KADYUSB340,i,&InterfaceData); i++)
	{
		//���Ȼ�ȡ��Ϣ����
		if (!SetupDiGetDeviceInterfaceDetail(hDeviceInfo,&InterfaceData,NULL,0,&nBufferSize,NULL))
		{
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) continue;
		}

		//���仺����
		pDeviceDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[nBufferSize];
		if(pDeviceDetail == NULL) continue;

		//���ó���
		pDeviceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		//�����豸��ϸ��Ϣ
		if (SetupDiGetDeviceInterfaceDetail(hDeviceInfo,&InterfaceData,pDeviceDetail,nBufferSize,NULL,NULL))
		{
			//������Ƶ���������
			m_arrDeviceName.Add(pDeviceDetail->DevicePath);
		}

		//ɾ��������
		delete pDeviceDetail;
	}

	//����豸��Ϣ
	SetupDiDestroyDeviceInfoList(hDeviceInfo);

	return TRUE;
}

//��ȡ�豸�ĸ���
int CKdUsb::GetDeviceCount()
{
	return (int)m_arrDeviceName.GetCount();
}

//��ȡ��ǰ�򿪵��豸
int CKdUsb::GetCurDevice()
{
	return m_nCurDevice;
}

//��ȡ�豸������
CString CKdUsb::GetDeviceName(int nIndex)
{
	if(nIndex < 0) return _T("");
	if(nIndex >= m_arrDeviceName.GetCount()) return _T("");

	return m_arrDeviceName.GetAt(nIndex);
}

//�ж��豸�Ƿ��
BOOL CKdUsb::IsOpen()
{
	if(m_hDevice == NULL) return FALSE;
	return TRUE;
}

//���豸
BOOL CKdUsb::Open(int nPort)
{
	if(nPort < 0) return FALSE;
	if(m_arrDeviceName.GetCount() <= nPort) return FALSE;
	
	CString DeviceName = m_arrDeviceName[nPort];

	m_hDevice = CreateFile(	DeviceName,
							GENERIC_WRITE | GENERIC_READ,
							FILE_SHARE_WRITE | FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							0,
							NULL);

	if((m_hDevice == INVALID_HANDLE_VALUE) || (m_hDevice == NULL)) 
	{
		m_hDevice = NULL;
		return FALSE;
	}

	m_nCurDevice = nPort;

	return TRUE;
}

//�ر��豸
BOOL CKdUsb::Close()
{
	if(m_hDevice != NULL)
	{
		CloseHandle(m_hDevice);
		m_hDevice = NULL;
	}

	m_nCurDevice = -1;

	return TRUE;	
}

//����
int CKdUsb::Send(char* pData, int nLen)
{
	int nRealSend;

	//����豸�Ƿ��Ѿ���
	if(m_hDevice == NULL) return 0;

	//���ݳ��ȼ��
	nLen = (nLen > EP2_PACKET_SIZE) ? EP2_PACKET_SIZE : nLen;

	//��������
	if(WriteFile(m_hDevice, pData, nLen, (LPDWORD)(&nRealSend), NULL) == TRUE)
	{
		return nRealSend;
	}

	return 0;	
}

//����
int CKdUsb::Recvive(char* pData, int nLen)
{
	int nRealRecv;

	//����豸�Ƿ��Ѿ���
	if(m_hDevice == NULL) return 0;

	//���ݳ��ȼ��
	nLen = (nLen > EP1_PACKET_SIZE) ? EP1_PACKET_SIZE : nLen;

	//��������
	if(ReadFile(m_hDevice, pData, nLen, (LPDWORD)(&nRealRecv), NULL) == TRUE)
	{
		return nRealRecv;
	}

	return 0;
}


