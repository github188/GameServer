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
	{//�رտ��Ź�
		ClosedWatchDog();
		return;
	}

	//���Ź�������
	if (m_hWatchDog)
	{
		return;
	}

	//�������Ź�
	//��һ���ӿں����������������
	if(OpenWatchDog(6))
	{
		//���ݷ���������pc�ĵ�½�û��������룬Ϊ�Զ���������
		//if (!WriteAdminInfoToFileEX(m_chSysName, m_chSysPassWord))
		//{
		//	AfxMessageBox(_T("�û���Ϣ����ʧ��!"), MB_OK | MB_ICONINFORMATION);
		//}

		//���������������̺߳Ϳ����߳��˳����¼�
		DWORD dwRet;
		m_hCloseWatchDogEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hWatchDog = CreateThread(NULL,0, LPTHREAD_START_ROUTINE (Thread_WatchDog_HeartBeat), (void*)this,0,&dwRet);


		LOG(LEVEL_INFO, "�������Ź�����");

		if (m_hWatchDog && bNotice)
		{
			AfxMessageBox(_T("�������Ź��ɹ�!"), MB_ICONINFORMATION);
		}

	}
	else 
	{
		AfxMessageBox(_T("�������Ź�ʧ��!"),MB_ICONSTOP);
	}

}

void WatchDog::ClosedWatchDog()
{
	//�رտ��Ź�
	if (m_hWatchDog != NULL)
	{ 	
		SetEvent(m_hCloseWatchDogEvent);

		if( WaitForSingleObject(m_hWatchDog, INFINITE) == WAIT_OBJECT_0)
		{
			CloseHandle(m_hWatchDog);
			m_hWatchDog = NULL;

			CloseHandle(m_hCloseWatchDogEvent);
			m_hCloseWatchDogEvent = NULL;

			//�����Ź����������˳������������������������˳��������쳣�˳����������Ź��Ͳ�����������pc��
			SendHeartbeat(DOG_EXIT);
		}

		LOG(LEVEL_INFO, "�رտ��Ź�����");
	}
}

DWORD WINAPI WatchDog::Thread_WatchDog_HeartBeat(LPVOID pParam)//zhouzx
{
	WatchDog * pThis = (WatchDog *)pParam;
	DOG_STATUS struDogStatus;

	for(;;)
	{
		//�����Ź����������������ӿ�
		SendHeartbeat(DOG_NORMAL);

		//�����̲�����ʱ����������
		struDogStatus = GetWatchdogStatus();
		if (DOG_NOTACTIVE == struDogStatus)
		{
			//TRACE("==YQ== ����������!!!");
			OpenWatchDog(6);
		}

		//���Ʒ���������ʱ����
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
	//�õ����������ȫ·��   
	DWORD dwRet				  = GetModuleFileName(NULL, pFileName, MAX_PATH); 
	HKEY RootKey			  = HKEY_LOCAL_MACHINE;									//ע�����������   
	char szSubKey[MAX_PATH]	  = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";//����ע���ֵ�ĵ�ַ   
	char szValueName[MAX_PATH]= "BayonetServer.exe";										//������ֵ������ 	

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
