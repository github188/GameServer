// GameServer.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "GameServer.h"
#include "GameServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGameServerApp

BEGIN_MESSAGE_MAP(CGameServerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CGameServerApp 构造

CGameServerApp::CGameServerApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CGameServerApp 对象

CGameServerApp theApp;

// CGameServerApp 初始化

BOOL CGameServerApp::InitInstance()
{

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();
	AfxEnableControlContainer();

	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

    m_hMutex = ::CreateMutex(NULL, FALSE, _T(GIS_MSG));
    if(ERROR_ALREADY_EXISTS == GetLastError())	
    {	
        DWORD dwRecipients = BSM_APPLICATIONS; 
        ::BroadcastSystemMessage(BSF_NOHANG, &dwRecipients, ID_GIS_INSTANCE, 0, 0);
        return FALSE;	
    }

    m_pADOOperation = new CADOOperation();

	CGameServerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

int CGameServerApp::ExitInstance()
{
    delete m_pADOOperation;
    return CWinApp::ExitInstance();
}
