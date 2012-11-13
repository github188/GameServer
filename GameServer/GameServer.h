// GameServer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#include "UnhandledExceptionFilter.h"


#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

#define GIS_MSG "{F952E14D-394A-4940-9A4B-C496B353BC12}"
const UINT ID_GIS_INSTANCE = ::RegisterWindowMessage(GIS_MSG); //ע����Ϣ

// CGameServerApp:
// �йش����ʵ�֣������ GameServer.cpp
//

class CGameServerApp : public CWinApp, public CUnhandledExceptionFilter
{
public:
	CGameServerApp();

// ��д
	public:
	virtual BOOL InitInstance();
    virtual int ExitInstance();

    CADOOperation* m_pADOOperation;

// ʵ��

	DECLARE_MESSAGE_MAP()

private:
    HANDLE m_hMutex;
};

extern CGameServerApp theApp;