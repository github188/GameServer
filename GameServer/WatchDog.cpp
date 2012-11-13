#include "stdafx.h"
#include "WatchDog.h"
#include "WatchDogInterface.h"

WatchDog::WatchDog()
{
	m_bOpenWatchDog = false;
	memset(&m_chSysName,0,100*sizeof(char));
	memset(&m_chSysPassWord,0,100*sizeof(char));
	m_hCloseWatchDogEvent = NULL;
	m_hWatchDog = NULL;
}

void WatchDog::RunWatchDog(bool bNotice)
{
	if (false == m_bOpenWatchDog)
	{//关闭看门狗
		ClosedWatchDog();
		return;
	}

	//看门狗已启动
	if (m_hWatchDog)
	{
		return;
	}

	//启动看门狗
	//第一个接口函数，传递心跳间隔
	if(OpenWatchDog(6))
	{
		//传递服务器所在pc的登陆用户名和密码，为自动重启所用
		//if (!WriteAdminInfoToFileEX(m_chSysName, m_chSysPassWord))
		//{
		//	AfxMessageBox(_T("用户信息保存失败!"), MB_OK | MB_ICONINFORMATION);
		//}

		//创建发送心跳的线程和控制线程退出的事件
		DWORD dwRet;
		m_hCloseWatchDogEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hWatchDog = CreateThread(NULL,0, LPTHREAD_START_ROUTINE (Thread_WatchDog_HeartBeat), (void*)this,0,&dwRet);


		LOG(LEVEL_INFO, "开启看门狗程序");

		if (m_hWatchDog && bNotice)
		{
			AfxMessageBox(_T("启动看门狗成功!"), MB_ICONINFORMATION);
		}

	}
	else 
	{
		AfxMessageBox(_T("启动看门狗失败!"),MB_ICONSTOP);
	}

}

void WatchDog::ClosedWatchDog()
{
	//关闭看门狗
	if (m_hWatchDog != NULL)
	{ 	
		SetEvent(m_hCloseWatchDogEvent);

		if( WaitForSingleObject(m_hWatchDog, INFINITE) == WAIT_OBJECT_0)
		{
			CloseHandle(m_hWatchDog);
			m_hWatchDog = NULL;

			CloseHandle(m_hCloseWatchDogEvent);
			m_hCloseWatchDogEvent = NULL;

			//给看门狗发送正常退出的心跳，表明程序是正常退出，而非异常退出。这样看门狗就不会重新启动pc。
			SendHeartbeat(DOG_EXIT);
		}

		LOG(LEVEL_INFO, "关闭看门狗程序");
	}
}

DWORD WINAPI WatchDog::Thread_WatchDog_HeartBeat(LPVOID pParam)//zhouzx
{
	WatchDog * pThis = (WatchDog *)pParam;
	DOG_STATUS struDogStatus;

	for(;;)
	{
		//给看门狗发送正常的心跳接口
		SendHeartbeat(DOG_NORMAL);

		//狗进程不存在时重启狗进程
		struDogStatus = GetWatchdogStatus();
		if (DOG_NOTACTIVE == struDogStatus)
		{
			//TRACE("==YQ== 狗进程重启!!!");
			OpenWatchDog(6);
		}

		//控制发送心跳的时间间隔
		if(WaitForSingleObject(pThis->m_hCloseWatchDogEvent,6000) == WAIT_OBJECT_0)
		{
			break;
		}
	}

	return 0;
}

BOOL WatchDog::WriteAutoRun(bool bAutoRun)
{
	char pFileName[MAX_PATH] = {0};   
	//得到程序自身的全路径   
	DWORD dwRet				  = GetModuleFileName(NULL, pFileName, MAX_PATH); 
	HKEY RootKey			  = HKEY_LOCAL_MACHINE;									//注册表主键名称   
	char szSubKey[MAX_PATH]	  = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";//欲打开注册表值的地址   
	char szValueName[MAX_PATH]= "BayonetServer.exe";										//欲设置值的名称 	

	if (bAutoRun)
	{
		if (!SetValue(RootKey, szSubKey, szValueName, (BYTE*)pFileName))   
		{
			return FALSE;
		}
	}else
	{
		if(!DeleteValue(RootKey, szSubKey, szValueName))   
		{
			return FALSE;   
		}
	}



	return TRUE;
}

BOOL WatchDog::SetValue (HKEY__ *ReRootKey, TCHAR *ReSubKey, TCHAR *ReValueName, LPBYTE ReSetContent_S)   
{   
	HKEY hKey;  
	if(RegOpenKeyEx(ReRootKey, ReSubKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)   
	{   
		if(RegSetValueEx(hKey, ReValueName, NULL, REG_SZ, ReSetContent_S, CString(ReSetContent_S).GetLength()) != ERROR_SUCCESS)   
		{   
			RegCloseKey(hKey); 
			return FALSE;   
		}   
		RegCloseKey(hKey);   
	}else   
	{   
		return FALSE;   
	}   
	return TRUE;   
}


BOOL WatchDog::DeleteValue (HKEY__ *ReRootKey, TCHAR *ReSubKey, TCHAR *ReValueName)   
{   
	HKEY hKey;
	if(RegOpenKeyEx(ReRootKey, ReSubKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)   
	{   
		if(RegDeleteValue(hKey, ReValueName) != ERROR_SUCCESS)   
		{   
			RegCloseKey(hKey); 
			return FALSE;   
		} 

		RegCloseKey(hKey);   
	}else   
	{   
		return FALSE;   
	}   
	return TRUE;    
}
