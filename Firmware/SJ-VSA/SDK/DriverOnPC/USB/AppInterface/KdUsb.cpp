#include "StdAfx.h"
#include "KdUsb.h"

//头文件
#include "usb100.h"
#include <initguid.h>
#include <setupapi.h>
#pragma comment (lib, "setupapi.lib")

//数据收发最大长度定义========================================================================
//注意：这下面两个参数一定要和设备上保持一致，或者更小，否则会出错
#define  EP1_PACKET_SIZE         	0x0040      //端点1数据区长度，用于发送数据到主机(最大128字节)
#define  EP2_PACKET_SIZE         	0x0040      //端点2数据区长度，用于从主机接收数据(最大256字节)


//设备类全局ID定义============================================================================
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

//枚举设备
BOOL CKdUsb::EnumDevices()
{
	//变量定义
	LONG i = 0;
	DWORD nBufferSize;
	SP_DEVICE_INTERFACE_DETAIL_DATA* pDeviceDetail = NULL;

	//清除名称数组
	m_arrDeviceName.RemoveAll();

	//寻找与驱动匹配的设备
	HDEVINFO hDeviceInfo = SetupDiGetClassDevs(
												(LPGUID)&GUID_DEVINTERFACE_KADYUSB340,
												NULL,
												NULL,
												DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
												);
	if(hDeviceInfo == INVALID_HANDLE_VALUE) return FALSE;

	//接口数据
	SP_DEVICE_INTERFACE_DATA InterfaceData;
	InterfaceData.cbSize = sizeof(InterfaceData);

	//枚举设备
	for(i = 0; SetupDiEnumDeviceInterfaces(hDeviceInfo,NULL,(LPGUID)&GUID_DEVINTERFACE_KADYUSB340,i,&InterfaceData); i++)
	{
		//首先获取信息长度
		if (!SetupDiGetDeviceInterfaceDetail(hDeviceInfo,&InterfaceData,NULL,0,&nBufferSize,NULL))
		{
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) continue;
		}

		//分配缓冲区
		pDeviceDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[nBufferSize];
		if(pDeviceDetail == NULL) continue;

		//设置长度
		pDeviceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		//设置设备详细信息
		if (SetupDiGetDeviceInterfaceDetail(hDeviceInfo,&InterfaceData,pDeviceDetail,nBufferSize,NULL,NULL))
		{
			//添加名称到名称数组
			m_arrDeviceName.Add(pDeviceDetail->DevicePath);
		}

		//删除缓冲区
		delete pDeviceDetail;
	}

	//清除设备信息
	SetupDiDestroyDeviceInfoList(hDeviceInfo);

	return TRUE;
}

//获取设备的个数
int CKdUsb::GetDeviceCount()
{
	return (int)m_arrDeviceName.GetCount();
}

//获取当前打开的设备
int CKdUsb::GetCurDevice()
{
	return m_nCurDevice;
}

//获取设备的名称
CString CKdUsb::GetDeviceName(int nIndex)
{
	if(nIndex < 0) return _T("");
	if(nIndex >= m_arrDeviceName.GetCount()) return _T("");

	return m_arrDeviceName.GetAt(nIndex);
}

//判断设备是否打开
BOOL CKdUsb::IsOpen()
{
	if(m_hDevice == NULL) return FALSE;
	return TRUE;
}

//打开设备
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

//关闭设备
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

//发送
int CKdUsb::Send(char* pData, int nLen)
{
	int nRealSend;

	//检查设备是否已经打开
	if(m_hDevice == NULL) return 0;

	//数据长度检查
	nLen = (nLen > EP2_PACKET_SIZE) ? EP2_PACKET_SIZE : nLen;

	//发送数据
	if(WriteFile(m_hDevice, pData, nLen, (LPDWORD)(&nRealSend), NULL) == TRUE)
	{
		return nRealSend;
	}

	return 0;	
}

//接收
int CKdUsb::Recvive(char* pData, int nLen)
{
	int nRealRecv;

	//检查设备是否已经打开
	if(m_hDevice == NULL) return 0;

	//数据长度检查
	nLen = (nLen > EP1_PACKET_SIZE) ? EP1_PACKET_SIZE : nLen;

	//接收数据
	if(ReadFile(m_hDevice, pData, nLen, (LPDWORD)(&nRealRecv), NULL) == TRUE)
	{
		return nRealRecv;
	}

	return 0;
}


