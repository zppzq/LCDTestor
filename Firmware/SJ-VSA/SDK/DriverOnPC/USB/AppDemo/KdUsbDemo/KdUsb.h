#ifndef _KD_USB_H_
#define _KD_USB_H_

class CKdUsb
{
public:
	CKdUsb(void);
	virtual ~CKdUsb(void);

private:
	int m_nCurDevice;						//��ǰʹ�õ��豸
	CStringArray m_arrDeviceName;			//�豸����
	HANDLE m_hDevice;						//�豸���
	
public:
	//��ʼ��
	BOOL EnumDevices();						//ö���豸
	
	//����
	int  GetDeviceCount();					//��ȡ�豸�ĸ���
	int  GetCurDevice();					//��ȡ��ǰ�򿪵��豸
	CString GetDeviceName(int nIndex);		//��ȡ�豸������
	BOOL IsOpen();							//�ж��豸�Ƿ��

	//ͨ��
	BOOL Open(int nPort = 0);				//���豸
	BOOL Close();							//�ر��豸
	int  Send(char* pData, int nLen);		//����
	int  Recvive(char* pData, int nLen);	//����
};


#endif
