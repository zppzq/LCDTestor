#ifndef _KD_USB_H_
#define _KD_USB_H_

class CKdUsb
{
public:
	CKdUsb(void);
	virtual ~CKdUsb(void);

private:
	int m_nCurDevice;						//当前使用的设备
	CStringArray m_arrDeviceName;			//设备名称
	HANDLE m_hDevice;						//设备句柄
	
public:
	//初始化
	BOOL EnumDevices();						//枚举设备
	
	//属性
	int  GetDeviceCount();					//获取设备的个数
	int  GetCurDevice();					//获取当前打开的设备
	CString GetDeviceName(int nIndex);		//获取设备的名称
	BOOL IsOpen();							//判断设备是否打开

	//通信
	BOOL Open(int nPort = 0);				//打开设备
	BOOL Close();							//关闭设备
	int  Send(char* pData, int nLen);		//发送
	int  Recvive(char* pData, int nLen);	//接收
};


#endif
