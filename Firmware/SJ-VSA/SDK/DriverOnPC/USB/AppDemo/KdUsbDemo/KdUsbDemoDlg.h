// KdUsbDemoDlg.h : ͷ�ļ�
//

#pragma once

#include "KdUsb.h"
class CKdUsbDemoDlg : public CDialog
{
// ����
public:
	CKdUsbDemoDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_KDUSBDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
public:
	afx_msg void OnBnClickedSend();
public:
	CString m_strDataSend;
public:
	CString m_strDataRecv;

private:
	CKdUsb m_Usb;
};
