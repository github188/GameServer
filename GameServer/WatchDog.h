#ifndef WatchDog_h__
#define WatchDog_h__

#include "UnhandledExceptionFilter.h"
#pragma comment(lib, "WatchDogDll.lib")

class WatchDog
{
public:
    WatchDog();

    void RunWatchDog(bool bNotice = false);
    void ClosedWatchDog();

    static DWORD WINAPI Thread_WatchDog_HeartBeat(LPVOID pParam);

    HANDLE m_hCloseWatchDogEvent;
    HANDLE m_hWatchDog;

    char m_chSysName[100];  //ϵͳ�û���
    char m_chSysPassWord[100];  //ϵͳ����
    bool m_bOpenWatchDog;       //�������Ź���״̬���λ

    //���ÿ���������
    BOOL WriteAutoRun(bool bAutoRun);
    //���ÿ���������дע���
    BOOL SetValue (HKEY__*ReRootKey, TCHAR *ReSubKey, TCHAR *ReValueName, LPBYTE ReSetContent_S);
    //ȡ������������дע���
    BOOL DeleteValue (HKEY__*ReRootKey, TCHAR *ReSubKey, TCHAR *ReValueName); 
};

#endif // WatchDog_h__

