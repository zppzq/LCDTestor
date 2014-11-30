// KdUsbDemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "KdUsbDemo.h"
#include "KdUsbDemoDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CKdUsbDemoDlg �Ի���
CKdUsbDemoDlg::CKdUsbDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKdUsbDemoDlg::IDD, pParent)
	, m_strDataSend(_T(""))
	, m_strDataRecv(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CKdUsbDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DATA_RECV, m_strDataRecv);
	DDX_Text(pDX, IDC_DATA_SEND, m_strDataSend);
}

BEGIN_MESSAGE_MAP(CKdUsbDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CKdUsbDemoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_SEND, &CKdUsbDemoDlg::OnBnClickedSend)
END_MESSAGE_MAP()


// CKdUsbDemoDlg ��Ϣ�������
BOOL CKdUsbDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	// ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	//ͨ�ų�ʼ��
	m_Usb.EnumDevices();
	m_Usb.Open();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CKdUsbDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

//	�����Ի��������С����ť������Ҫ����Ĵ���
//	�����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//	�⽫�ɿ���Զ���ɡ�
void CKdUsbDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
HCURSOR CKdUsbDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CKdUsbDemoDlg::OnBnClickedOk()
{
	//�򿪶˿�
}

void CKdUsbDemoDlg::OnBnClickedSend()
{
	int nLen;						//���ݳ���
	char RecvBuff[200];				//���ջ�����

	memset(RecvBuff, 0, 200);		//������ջ�����

	UpdateData(TRUE);				//��������

	nLen = m_Usb.Send((char*)m_strDataSend.GetBuffer(), m_strDataSend.GetLength());		//��������
	nLen = m_Usb.Recvive(RecvBuff, m_strDataSend.GetLength());							//��������

	//��ʾ����
	m_strDataRecv.Format(_T("%s"), RecvBuff);
	UpdateData(FALSE);
}
