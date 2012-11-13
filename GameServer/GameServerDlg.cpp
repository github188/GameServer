// GameServerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "GameServer.h"
#include "GameServerDlg.h"



//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

CGameServerDlg* g_pDlg = NULL;

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


// CGameServerDlg �Ի���




CGameServerDlg::CGameServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGameServerDlg::IDD, pParent)
    , m_hServerThread(NULL)
    , m_sPort(11543)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    g_pDlg = this;
}

void CGameServerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_LOG, m_LogList);
}

BEGIN_MESSAGE_MAP(CGameServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDOK, &CGameServerDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CGameServerDlg::OnBnClickedCancel)
    ON_REGISTERED_MESSAGE(ID_GIS_INSTANCE, OnGISInstance)
    ON_MESSAGE(WM_ICON_NOTIFY, OnNotifyIcon)
    ON_WM_TIMER()
    ON_NOTIFY(NM_RCLICK, IDC_LIST_LOG, &CGameServerDlg::OnNMRClickListLog)
END_MESSAGE_MAP()


// CGameServerDlg ��Ϣ�������

BOOL CGameServerDlg::OnInitDialog()
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
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    //SetWindowText(_T("΢�۽�ѧ������ build20120219"));

	Init();
    GetVersionInfo();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CGameServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    if(nID == SC_MINIMIZE)
    {

        ShowWindow(SW_HIDE); 
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CGameServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
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

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CGameServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CGameServerDlg::OnBnClickedOk()
{
    m_iocp.Stop();
}

void CGameServerDlg::OnBnClickedCancel()
{
    if(IDCANCEL == AfxMessageBox(_T("ȷ��Ҫ�رշ�������"), MB_OKCANCEL | MB_ICONQUESTION))
    {
        return;
    }

#ifndef _DEBUG
    m_WatchDog.ClosedWatchDog();
#endif
    SetEvent (m_hExitEvent);
    KillTimer(1);
    Shell_NotifyIcon(NIM_DELETE, &m_NoitfyIcon);
    m_iocp.Stop();
    if(NULL != m_hServerThread)
    {
        WaitForSingleObject(m_hServerThread, INFINITE);
        CloseHandle(m_hServerThread);
        m_hServerThread = NULL;
    }
    OnCancel();
}

void CGameServerDlg::Init()
{
    m_NoitfyIcon.cbSize = (DWORD)sizeof(NOTIFYICONDATA); 
    m_NoitfyIcon.hWnd = this->m_hWnd; 
    m_NoitfyIcon.uID = IDR_MAINFRAME; 
    m_NoitfyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP ; 
    m_NoitfyIcon.uCallbackMessage = WM_ICON_NOTIFY;
    m_NoitfyIcon.hIcon = m_hIcon;
    strcpy_s(m_NoitfyIcon.szTip, 128, "��Ϸ������"); 
    Shell_NotifyIcon(NIM_ADD, &m_NoitfyIcon); 

    m_LogList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
    m_LogList.InsertColumn(0, TEXT("��Ϣ"), LVCFMT_CENTER, 510);

    m_StartTime = CTime::GetCurrentTime();

    LoadConfig ();

    if(m_FlashSecurity.Start())
    {
        ShowLog("Flashɳ�������ɹ�");
    }
    else
    {
        ShowLog("Flashɳ������ʧ��");
    }

    InitDB();

    m_hServerThread = CreateThread(NULL, 0, ServerThread, this, 0, NULL);
    if(NULL == m_hServerThread)
    {
        ShowLog("����������ʧ�ܣ�����������");
    }
    else
    {
        ShowLog("�����������ɹ��������˿�:%d", m_sPort);
    }
    SetTimer(1, 1000, NULL);

    m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    CreateThread(NULL, 0, CheckThread, this, 0, NULL);

    //���Ź�
#ifndef _DEBUG
    m_WatchDog.m_bOpenWatchDog = true;
    m_WatchDog.RunWatchDog(false);
#endif
}


LRESULT CGameServerDlg::OnGISInstance(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    theApp.m_pMainWnd->ShowWindow(SW_SHOWNORMAL);
    theApp.m_pMainWnd->SetForegroundWindow();
    ::AfxMessageBox(_T("�������Ѿ������У�"));
    return 0;
}

DWORD WINAPI CGameServerDlg::ServerThread(LPVOID lParam)
{
    CGameServerDlg* pThis = (CGameServerDlg*)lParam;
    pThis->StartServer();
    LOG(LEVEL_OUTPUT, "���������߳��˳�");
    return 0;
}

void CGameServerDlg::StartServer(void)
{
    m_sPort = 11543;                 
    size_t io_pool_size = 16;
    size_t work_pool_init_size = 8;
    size_t work_pool_high_watermark = 32;
    size_t preallocated_handler_number = 50;
    size_t read_buffer_size = 1024;
    size_t write_buffer_size = 1024;
    size_t session_timeout = 60 * 35;
    size_t io_timeout = 35;

    LOG(LEVEL_OUTPUT, "����������,PORT:%d", m_sPort);
    m_iocp.Start(m_sPort, io_pool_size, work_pool_init_size, work_pool_high_watermark, 
        preallocated_handler_number, read_buffer_size, write_buffer_size, session_timeout, io_timeout);
}   

//˫��ϵͳ���̣�ʹ����������
LRESULT CGameServerDlg::OnNotifyIcon(WPARAM /*wParam*/, LPARAM lParam)
{
    switch (lParam)
    {
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
        {
            ShowWindow(SW_SHOWNORMAL); //��ʾ����
            SetForegroundWindow();
            break;
        }
    case WM_RBUTTONDOWN:
        {
            /*POINT pt;
            GetCursorPos(&pt);
            CMenu Menu;
            if (Menu.LoadMenu(IDR_MENU_NOTIFY))
            {
                if (CMenu * pTrayMenu = Menu.GetSubMenu(0))
                {
                    SetForegroundWindow();
                    pTrayMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
                }
                Menu.DestroyMenu();
            }*/
        }
    default:
        break;
    }

    return 0;
}

void CGameServerDlg::GetMemoryInfo()
{
    /*HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    if(NULL == hProcess)
    {
        return;
    }
    
    CString strTmp = "";
    PROCESS_MEMORY_COUNTERS pmc;
    pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
    if(GetProcessMemoryInfo(hProcess, &pmc, sizeof(PROCESS_MEMORY_COUNTERS)))
    {
        strTmp.Format("��ǰ�ڴ�ʹ��: %d K", pmc.WorkingSetSize / 1024);
        SetDlgItemText(IDC_STATIC_MEM, strTmp);
    }*/

}

void CGameServerDlg::OnTimer(UINT_PTR nIDEvent)
{
    int iItemCount = 0;
    m_LogVectorLock.Lock();
    for (UINT i = 0; i < m_LogVector.size(); i++)
    {
        iItemCount = m_LogList.GetItemCount();
        //��־�������Ϊ5000��ʱ���
        if (iItemCount > 5000)
        {
            m_LogList.DeleteAllItems();
            iItemCount = 0;
        }
        m_LogList.InsertItem(iItemCount, m_LogVector[i]);	
        //�Զ�����
        m_LogList.EnsureVisible(iItemCount, TRUE);
    }
    m_LogVector.clear();
    m_LogVectorLock.Unlock();

    CString strInfo = "";
    CTimeSpan timeSpan = CTime::GetCurrentTime() - m_StartTime;
    strInfo.Format("%s", timeSpan.Format("������������: %D�� %H:%M:%S"));
    SetDlgItemText(IDC_STATIC_RUNTIME, strInfo);

    if(NULL != m_hServerThread)
    {
        DWORD dwExitCode = 0;

        if (!GetExitCodeThread(m_hServerThread, &dwExitCode))
        {
            LOG(LEVEL_ERROR, "��ȡ�����߳��˳�Codeʧ��!");
        }
        else
        {
            if (STILL_ACTIVE != dwExitCode)
            {
                ShowLog("���������߳���ֹͣ������������������");
                LOG(LEVEL_ERROR, "���������߳���ֹͣ������������������");
                CloseHandle(m_hServerThread);
                m_hServerThread = NULL;
            }
        }
    }

    CDialog::OnTimer(nIDEvent);
}

void CGameServerDlg::ShowLog(LPCTSTR ptzFormat, ...)
{
    TCHAR tzText[1024] = "";
    va_list vlArgs;
    va_start(vlArgs, ptzFormat);
    wvsprintf(tzText, ptzFormat, vlArgs);
    va_end(vlArgs);

    CTime  nowTime = CTime::GetCurrentTime();
    CString strTmp = nowTime.Format("%Y-%m-%d %H:%M:%S");
    strTmp.AppendFormat(": %s", tzText);

    m_LogVectorLock.Lock();
    m_LogVector.push_back(strTmp);
    m_LogVectorLock.Unlock();
}

void CGameServerDlg::OnNMRClickListLog(NMHDR *pNMHDR, LRESULT *pResult)
{
    //LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE>(pNMHDR);
    
    CMenu menu;	
    CMenu* menuSub;
    CPoint point;
    GetCursorPos(&point);

    BOOL bRet = menu.LoadMenu(IDR_MENU_LOGLIST);
    if (!bRet)
    {
        return;
    }

    menuSub = menu.GetSubMenu(0);
    int iCmd = menuSub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this);	
    if (ID_M_LOGLIST_CLEAR == iCmd)
    {
        m_LogList.DeleteAllItems();
    }
    else
    {		
    }
    menu.DestroyMenu();

    *pResult = 0;
}

void CGameServerDlg::GetVersionInfo(void)
{
    CString strFilePath;
    TCHAR AppFullPath[_MAX_PATH] = {0};   
    GetModuleFileName(NULL, AppFullPath, _MAX_PATH);   
    CString strAppPath = AppFullPath;
    int iAppPosition = strAppPath.ReverseFind('\\');   
    strFilePath = strAppPath.Mid(0, iAppPosition+1); 

    CString strTemp = _T("");
    CTime tFileTime  = time(NULL);
    CString   strFile = _T("");
    CFileFind cFilefind;
    strFile.Format("%sGameServer.exe", strFilePath);

    if(cFilefind.FindFile(strFile))
    {
        cFilefind.FindNextFile();
        cFilefind.GetLastWriteTime(tFileTime);  
        strTemp.Format(_T("΢�۷����� Build %04d%02d%02d"), tFileTime.GetYear(),tFileTime.GetMonth(),tFileTime.GetDay());
        SetWindowText(strTemp);
    }else
    {
        SetWindowText("΢�۷����� Build20120301");
    }
    cFilefind.Close();
}

bool CGameServerDlg::InitDB()
{
    LOG(LEVEL_OUTPUT, "��ʼ�������ݿ⣬IP:%s DBName:%s, DBUser:%s, DBPwd:%s", 
        m_strDBServerIP, m_strDBName, m_strDBUser, m_strDBPwd);
    if(!theApp.m_pADOOperation->Init (m_strDBServerIP, m_strDBName, m_strDBUser, m_strDBPwd))
    {
        ShowLog("�������ݿ�ʧ��!");
        return FALSE;
    }
    ShowLog("�������ݿ�ɹ�!");
    return true;
}

DWORD WINAPI CGameServerDlg::CheckThread(LPVOID lParam)
{
    CGameServerDlg* pThis = (CGameServerDlg*)lParam;
    pThis->CheckProc ();
    return 0;
}

void CGameServerDlg::CheckProc(void)
{
    for (;;)
    {
        if(WAIT_OBJECT_0 == WaitForSingleObject (m_hExitEvent, 5000))
        {
            break;
        }
        m_iocp.check_timeout();   
    }
}

void CGameServerDlg::LoadConfig()
{
    m_strConfigPath = GetConfigPath ();
    char cTmp[256] = {0};

    GetPrivateProfileString("CONFIG", "DBServerIP", "", cTmp, 256, m_strConfigPath);
    m_strDBServerIP = cTmp;

    GetPrivateProfileString("CONFIG", "DBName", "", cTmp, 256, m_strConfigPath);
    m_strDBName = cTmp;

    GetPrivateProfileString("CONFIG", "DBUser", "", cTmp, 256, m_strConfigPath);
    m_strDBUser = cTmp;

    GetPrivateProfileString("CONFIG", "DBPwd", "", cTmp, 256, m_strConfigPath);
    m_strDBPwd = cTmp;

    //nValue = GetPrivateProfileInt(_T("CONFIG"), lpszName, 0, m_strConfigPath);
}

CString CGameServerDlg::GetConfigPath()
{
    //��ȡ�����ļ�·��
    CString strFilePath;
    TCHAR   AppFullPath[_MAX_PATH];   
    GetModuleFileName(NULL,AppFullPath,_MAX_PATH);   
    CString   strAppPath;   
    strAppPath = AppFullPath;
    int   iAppPosition;   
    iAppPosition=strAppPath.ReverseFind('\\');   
    strFilePath = strAppPath.Mid(0,iAppPosition+1); 
    CString strFileName = _T("Config.ini");
    return strFilePath + strFileName; 
}
