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

    char m_chSysName[100];  //系统用户名
    char m_chSysPassWord[100];  //系统密码
    bool m_bOpenWatchDog;       //启动看门狗的状态标记位

    //设置开机自启动
    BOOL WriteAutoRun(bool bAutoRun);
    //设置开机自启动写注册表
    BOOL SetValue (HKEY__*ReRootKey, TCHAR *ReSubKey, TCHAR *ReValueName, LPBYTE ReSetContent_S);
    //取消开机自启动写注册表
    BOOL DeleteValue (HKEY__*ReRootKey, TCHAR *ReSubKey, TCHAR *ReValueName); 
};

#endif // WatchDog_h__

