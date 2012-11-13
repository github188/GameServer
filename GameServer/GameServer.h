// GameServer.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#include "UnhandledExceptionFilter.h"


#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

#define GIS_MSG "{F952E14D-394A-4940-9A4B-C496B353BC12}"
const UINT ID_GIS_INSTANCE = ::RegisterWindowMessage(GIS_MSG); //注册消息

// CGameServerApp:
// 有关此类的实现，请参阅 GameServer.cpp
//

class CGameServerApp : public CWinApp, public CUnhandledExceptionFilter
{
public:
	CGameServerApp();

// 重写
	public:
	virtual BOOL InitInstance();
    virtual int ExitInstance();

    CADOOperation* m_pADOOperation;

// 实现

	DECLARE_MESSAGE_MAP()

private:
    HANDLE m_hMutex;
};

extern CGameServerApp theApp;