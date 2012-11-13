// GameServer.cpp : ����Ӧ�ó��������Ϊ��
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


// CGameServerApp ����

CGameServerApp::CGameServerApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CGameServerApp ����

CGameServerApp theApp;

// CGameServerApp ��ʼ��

BOOL CGameServerApp::InitInstance()
{

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();
	AfxEnableControlContainer();

	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

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
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

int CGameServerApp::ExitInstance()
{
    delete m_pADOOperation;
    return CWinApp::ExitInstance();
}
