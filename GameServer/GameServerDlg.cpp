// GameServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GameServer.h"
#include "GameServerDlg.h"



//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

CGameServerDlg* g_pDlg = NULL;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CGameServerDlg 对话框




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


// CGameServerDlg 消息处理程序

BOOL CGameServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

    //SetWindowText(_T("微舟教学服务器 build20120219"));

	Init();
    GetVersionInfo();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGameServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
    if(IDCANCEL == AfxMessageBox(_T("确定要关闭服务器？"), MB_OKCANCEL | MB_ICONQUESTION))
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
    strcpy_s(m_NoitfyIcon.szTip, 128, "游戏服务器"); 
    Shell_NotifyIcon(NIM_ADD, &m_NoitfyIcon); 

    m_LogList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
    m_LogList.InsertColumn(0, TEXT("消息"), LVCFMT_CENTER, 510);

    m_StartTime = CTime::GetCurrentTime();

    LoadConfig ();

    if(m_FlashSecurity.Start())
    {
        ShowLog("Flash沙箱启动成功");
    }
    else
    {
        ShowLog("Flash沙箱启动失败");
    }

    InitDB();

    m_hServerThread = CreateThread(NULL, 0, ServerThread, this, 0, NULL);
    if(NULL == m_hServerThread)
    {
        ShowLog("服务器启动失败，请重新启动");
    }
    else
    {
        ShowLog("服务器启动成功，监听端口:%d", m_sPort);
    }
    SetTimer(1, 1000, NULL);

    m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    CreateThread(NULL, 0, CheckThread, this, 0, NULL);

    //看门狗
#ifndef _DEBUG
    m_WatchDog.m_bOpenWatchDog = true;
    m_WatchDog.RunWatchDog(false);
#endif
}


LRESULT CGameServerDlg::OnGISInstance(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    theApp.m_pMainWnd->ShowWindow(SW_SHOWNORMAL);
    theApp.m_pMainWnd->SetForegroundWindow();
    ::AfxMessageBox(_T("服务器已经在运行！"));
    return 0;
}

DWORD WINAPI CGameServerDlg::ServerThread(LPVOID lParam)
{
    CGameServerDlg* pThis = (CGameServerDlg*)lParam;
    pThis->StartServer();
    LOG(LEVEL_OUTPUT, "服务器主线程退出");
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

    LOG(LEVEL_OUTPUT, "服务器启动,PORT:%d", m_sPort);
    m_iocp.Start(m_sPort, io_pool_size, work_pool_init_size, work_pool_high_watermark, 
        preallocated_handler_number, read_buffer_size, write_buffer_size, session_timeout, io_timeout);
}   

//双击系统托盘，使程序正常化
LRESULT CGameServerDlg::OnNotifyIcon(WPARAM /*wParam*/, LPARAM lParam)
{
    switch (lParam)
    {
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
        {
            ShowWindow(SW_SHOWNORMAL); //显示窗体
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
        strTmp.Format("当前内存使用: %d K", pmc.WorkingSetSize / 1024);
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
        //日志输出条数为5000条时清空
        if (iItemCount > 5000)
        {
            m_LogList.DeleteAllItems();
            iItemCount = 0;
        }
        m_LogList.InsertItem(iItemCount, m_LogVector[i]);	
        //自动滚动
        m_LogList.EnsureVisible(iItemCount, TRUE);
    }
    m_LogVector.clear();
    m_LogVectorLock.Unlock();

    CString strInfo = "";
    CTimeSpan timeSpan = CTime::GetCurrentTime() - m_StartTime;
    strInfo.Format("%s", timeSpan.Format("服务器已运行: %D天 %H:%M:%S"));
    SetDlgItemText(IDC_STATIC_RUNTIME, strInfo);

    if(NULL != m_hServerThread)
    {
        DWORD dwExitCode = 0;

        if (!GetExitCodeThread(m_hServerThread, &dwExitCode))
        {
            LOG(LEVEL_ERROR, "获取工作线程退出Code失败!");
        }
        else
        {
            if (STILL_ACTIVE != dwExitCode)
            {
                ShowLog("服务器主线程已停止工作，请重启服务器");
                LOG(LEVEL_ERROR, "服务器主线程已停止工作，请重启服务器");
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
        strTemp.Format(_T("微舟服务器 Build %04d%02d%02d"), tFileTime.GetYear(),tFileTime.GetMonth(),tFileTime.GetDay());
        SetWindowText(strTemp);
    }else
    {
        SetWindowText("微舟服务器 Build20120301");
    }
    cFilefind.Close();
}

bool CGameServerDlg::InitDB()
{
    LOG(LEVEL_OUTPUT, "开始连接数据库，IP:%s DBName:%s, DBUser:%s, DBPwd:%s", 
        m_strDBServerIP, m_strDBName, m_strDBUser, m_strDBPwd);
    if(!theApp.m_pADOOperation->Init (m_strDBServerIP, m_strDBName, m_strDBUser, m_strDBPwd))
    {
        ShowLog("连接数据库失败!");
        return FALSE;
    }
    ShowLog("连接数据库成功!");
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
    //获取配置文件路径
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
