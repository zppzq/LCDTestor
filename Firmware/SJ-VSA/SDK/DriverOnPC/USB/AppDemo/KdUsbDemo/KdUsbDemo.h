// KdUsbDemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CKdUsbDemoApp:
// �йش����ʵ�֣������ KdUsbDemo.cpp
//

class CKdUsbDemoApp : public CWinApp
{
public:
	CKdUsbDemoApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CKdUsbDemoApp theApp;