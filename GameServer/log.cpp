
#include "stdafx.h"
#include "log.h"
#include "time.h"
#include <fcntl.h>
#include <io.h>
#include <Share.h>
#include <SYS\Stat.h>

#include <Shlwapi.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <list>
#include <algorithm>
using namespace std;

static Log g_log;


/**   @fn      Log::Log()
 *    @brief   Log类的构造函数
 *    @param   无.
 *    @return  无
 */
Log::Log()
{
	TCHAR szPath[MAX_PATH] = {0};
    GetModuleFileName(NULL, szPath, MAX_PATH - 1);
    (_tcsrchr(szPath,'\\'))[1] = 0;
    sprintf(m_szLogDir, "%sLogs", szPath);

    if (!PathIsDirectory(m_szLogDir))
    {
        CreateDirectory(m_szLogDir, NULL);
    }

    sprintf(m_ConfigFile, "%sLog.Config", szPath);

    if (!PathIsDirectory(m_ConfigFile))
    {
        CreateDirectory(m_ConfigFile, NULL);
    }

    sprintf(m_ConfigFile, "%s\\%s", m_ConfigFile, CONFIG_FILE);

    LocateConfigFile();

    InitializeCriticalSection(&m_CriticalSection);

    InitLogging();

    m_hEven = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (INVALID_HANDLE_VALUE == m_hEven)
    {
        return;
    }

    m_hDleEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (INVALID_HANDLE_VALUE == m_hDleEvent)
	{
		return;
	}
    m_hThread = CreateThread(NULL, 0, Log::Monitor, this, NULL, NULL);
    if (INVALID_HANDLE_VALUE == m_hThread)
    {
        return;
    }

	m_hDleFileThread = CreateThread(NULL, 0, Log::DleFileFun, this, NULL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDleFileThread)
	{
		return;
	}
}

Log::~Log()
{
    ShutdownLogging();
}

/**   @fn     Log* Log::GetInstance()
 *    @brief  获取日志单例的实例   
 *    @param  无.
 *    @return 实例指针 
 */
Log* Log::GetInstance()
{
    return &g_log;
}

/**   @fn     DWORD Log::LocateConfigFile()
 *    @brief  定位配置文件,创建相关目录并初始化日志文件.
 *    @param  无.
 *    @return 无
 */
void Log::LocateConfigFile()
{
    WIN32_FIND_DATA wfd;
    HANDLE hFind = FindFirstFile(m_ConfigFile, &wfd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        ::WritePrivateProfileString("Config", "LOG_ENABLE_FILE_LOGGING", "TRUE", m_ConfigFile);
        ::WritePrivateProfileString("Config", "LOG_ENABLE_DEBUGGER_LOGGING", "TRUE", m_ConfigFile);
        ::WritePrivateProfileString("Config", "LOG_ENABLE" , "TRUE", m_ConfigFile);
        ::WritePrivateProfileString("Config", "LogVMLevel", "3", m_ConfigFile);
        ::WritePrivateProfileString("Config", "LogFileSize", "1", m_ConfigFile);
		::WritePrivateProfileString("Config", "LogFileMaxNum", "20", m_ConfigFile);
    }

    if (INVALID_HANDLE_VALUE != hFind)
    {
        FindClose(hFind);
    }

    m_timeCfg = GetConfigTime();
}
/**   @fn     VOID Log::InitLogging()
 *    @brief  日志实例的初始化操作
 *    @param  无.
 *    @return 无
 */
VOID Log::InitLogging()
{
	LoadConfig();
    
	m_LogFileHandle = INVALID_HANDLE_VALUE;

    COleDateTime curTime = COleDateTime::GetCurrentTime();

    sprintf(m_szLogFileName, "%s\\%.3s_%s.LOG", m_szLogDir, DEFAULT_LOGFILE_NAME, (LPCTSTR)curTime.Format("%Y-%m-%d %H_%M_%S"));

    if ((m_LogFlags & LOG_ENABLE)
        && (m_LogFlags & LOG_ENABLE_FILE_LOGGING)
        && (m_LogFileHandle == INVALID_HANDLE_VALUE))
    {
        m_LogFileHandle = CreateFile(m_szLogFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL 
            | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		
        // Some other logging may be going on, try again with another file name
        if (m_LogFileHandle == INVALID_HANDLE_VALUE)
        {
            sprintf(m_szLogFileName, "%s\\%.3s_%s($).LOG", m_szLogDir, DEFAULT_LOGFILE_NAME, (LPCTSTR)curTime.Format("%Y-%m-%d_%H_%M_%S"));
            char* ptr = _tcsrchr(m_szLogFileName, '$');
            ptr[0] = '0';

            for(int i = 0; i < 10; i++)
            {
                m_LogFileHandle = CreateFileA(m_szLogFileName, GENERIC_WRITE,
                    FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL 
                    | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
                if (m_LogFileHandle != INVALID_HANDLE_VALUE)
                {
                    break;
                }
                *ptr = *ptr + 1;
            }
        }
    }
}

/**   @fn     DWORD GetConfigDWORD(char* FilePath, char* LogFlag, DWORD flag)
 *    @brief  根据给出的配置文件路径，对指定的配置信息进行读取，返回特定的值
 *    @param  FilePath 配置文件路径.
 *    @param  LogFlag  配置信息.
 *    @param  flag     配置信息对应的宏值.如果读取的是int类型的配置信息
                       （如信息级别LogVMLevel）则此参数不需输入
 *    @return 配置信息的读取结果
 */
DWORD Log::GetConfigDWORD(char* FilePath, char* LogFlag, DWORD flag)
{
    if(flag != 0xffffffff)  
    {
        char buff[20];	
        GetPrivateProfileString("Config", LogFlag, NULL, buff, sizeof(buff), FilePath);
        if(0 == strcmp(buff, "TRUE"))
        {
            return flag;
        }
        else if(0 == strcmp(buff, "FALSE"))
        {
            return 0;
        }
    }
    return GetPrivateProfileInt("Config", LogFlag, LEVEL_INFO, FilePath);
}

/**   @fn     VOID Log::LoadConfig()
 *    @brief  加载日志配置文件信息
 *    @param  无.
 *    @return 无
 */
VOID Log::LoadConfig()
{
    m_LogFlags = 0;

    m_LogVMLevel  = GetConfigDWORD(m_ConfigFile, "LogVMLevel");
    m_LogFileSize = GetConfigDWORD(m_ConfigFile, "LogFileSize");
	m_LogFileMaxNum = GetConfigDWORD(m_ConfigFile, "LogFileMaxNum");

    m_LogFlags |= GetConfigDWORD(m_ConfigFile, "LOG_ENABLE", LOG_ENABLE);
    m_LogFlags |= GetConfigDWORD(m_ConfigFile, "LOG_ENABLE_DEBUGGER_LOGGING", LOG_ENABLE_DEBUGGER_LOGGING);
    m_LogFlags |= GetConfigDWORD(m_ConfigFile, "LOG_ENABLE_FILE_LOGGING", LOG_ENABLE_FILE_LOGGING);
}

/**   @fn     bool Log::LoggingOn(DWORD level)
 *    @brief  判断日志功能是否可用，判断记录信息级别是否合法
 *    @param  level 记录信息的级别.
 *    @return 日志功能可用且记录信息级别合法，则返回TRUE;否则为FALSE
 */
bool Log::LoggingOn(DWORD level)
{
    return((m_LogFlags & LOG_ENABLE) && level <= m_LogVMLevel);
}

/**   @fn     VOID Log::LogSpew(DWORD level, const char* fmt, ...)
 *    @brief  将信息记录到文件前的准备操作，判断日志文件大小是否达到最大值，是则
              初始化一新日志文件并进入记录信息函数，否则直接进入记录信息函数
 *    @param  level 当前记录信息的级别.
 *    @param  fmt   记录信息的内容的格式.
 *    @return 无
 */
VOID Log::LogSpew(DWORD level, const char* fmt, ...)
{
    EnterCriticalSection(&m_CriticalSection);
    
    if (GetFileSize(m_LogFileHandle, NULL) > (m_LogFileSize * 1024*1024))
    {
        CloseHandle(m_LogFileHandle);
        InitLogging();
    }

    va_list args;
    va_start( args, fmt );
    LogSpewValist(level,fmt, args);

    LeaveCriticalSection(&m_CriticalSection);
}

/**   @fn     VOID Log::LogSpewValist(DWORD level, const char *fmt, va_list args)
 *    @brief  将信息记录到日志文件，同时输出到调试窗口
 *    @param  level 当前记录信息的级别.
 *    @param  fmt   记录信息的内容的格式.
 *    @param  args  信息内容的组成成员数组.
 *    @return 无
 */
VOID Log::LogSpewValist(DWORD level, const char *fmt, va_list args)
{
    if (!LoggingOn(level))
    {
        return;
    }

    const int BUFFERSIZE = 4096;
    char rgchBuffer[BUFFERSIZE] = {0};

    char *  pBuffer = &rgchBuffer[0];
    DWORD   buflen = 0;
    DWORD   written;

    static bool needsPrefix = true;

    if (needsPrefix)
    {
        timeb tb;
        ftime(&tb);
        CTime curTime(tb.time);
        CString strTime;
        strTime.Format("%04d-%02d-%02d %02d:%02d:%02d.%03d",curTime.GetYear(),curTime.GetMonth(),
            curTime.GetDay(),curTime.GetHour(),curTime.GetMinute(),curTime.GetSecond(),tb.millitm);
        
        buflen = sprintf(pBuffer, "[TID %04x: %s] ", GetCurrentThreadId(),(LPCTSTR)strTime);
    }

    int cCountWritten = _vsnprintf(&pBuffer[buflen], BUFFERSIZE-buflen, fmt, args );
    pBuffer[BUFFERSIZE-1] = 0;
    if (cCountWritten < 0) 
    {
        buflen = BUFFERSIZE - 1;
    } 
    else 
    {
        buflen += cCountWritten;
    }

    // Its a little late for this, but at least you wont continue
    // trashing your program...
    _ASSERTE((buflen < (DWORD) BUFFERSIZE) && "Log text is too long!") ;

#if !PLATFORM_UNIX
    //convert NL's to CR NL to fixup notepad
    const int BUFFERSIZE2 = BUFFERSIZE + 512;
    char rgchBuffer2[BUFFERSIZE2] = {0};
    char * pBuffer2 = &rgchBuffer2[0];

    BOOL bNLExist = FALSE;

    char *d = pBuffer2;
    for (char *p = pBuffer; *p != '\0'; p++)
    {
        if (*p == '\n') 
        {
            _ASSERTE(d < pBuffer2 + BUFFERSIZE2);
            *(d++) = '\r';

            bNLExist = TRUE;
        }

        _ASSERTE(d < pBuffer2 + BUFFERSIZE2);
        *(d++) = *p;
    }

    if (!bNLExist)
    {
        *(d++) = '\r';
        *(d++) = '\n';
    }

    *d = 0;

    buflen  = (DWORD)(d - pBuffer2);
    pBuffer = pBuffer2;
#endif // PLATFORM_UNIX

    if (m_LogFlags & LOG_ENABLE_FILE_LOGGING && m_LogFileHandle != INVALID_HANDLE_VALUE)
    {
		if (WriteFile(m_LogFileHandle, pBuffer, buflen, &written, NULL) == 0)
		{
			OutputDebugStringA("write file fail....");
		}
    }

    if (m_LogFlags & LOG_ENABLE_DEBUGGER_LOGGING)
    {
        OutputDebugStringA(pBuffer);
    }
}

/**   @fn     DWORD WINAPI Log::Monitor(LPVOID lpParameter)
 *    @brief  监控配置文件变动的线程函数
 *    @param  lpParameter 线程函数的传入参数.
 *    @return 正常退出为0，否则为非0整数
 */
DWORD WINAPI Log::Monitor(LPVOID lpParameter)
{
    Log* log = (Log*)lpParameter;

    char Path[MAX_PATH];
    strcpy(Path, log->m_ConfigFile);
    (_tcsrchr(Path, '\\'))[1] = 0;

    HANDLE dwChangeHandle = FindFirstChangeNotification(Path, TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (dwChangeHandle == INVALID_HANDLE_VALUE)
    {
        CloseHandle(dwChangeHandle);
        OutputDebugStringA("Create FindFirstChangeNotification Handle Fail.");
        return GetLastError();
    }

    HANDLE handles[2];
    handles[0] = dwChangeHandle;
    handles[1] = log->m_hEven;

   for(;;)
    {
        DWORD dwWaitStatus = WaitForMultipleObjects(sizeof(handles)/sizeof(HANDLE), handles, FALSE, INFINITE);
        if (dwWaitStatus == WAIT_OBJECT_0)
        {
            if (log->ConfigFileIsWrite())
            {
                log->LoadConfig();

                int fd = 0;

                fd = _sopen(log->m_ConfigFile, _O_RDWR | _O_BINARY,
                    _SH_DENYNO, _S_IREAD | _S_IWRITE);

                char buff[CONFIG_FILE_SIZE] = {0};
                _read(fd, buff, CONFIG_FILE_SIZE);
                LOG(LEVEL_FATALERROR,"Config Changed\n%s\n",buff);
                _close(fd);
            }

            FindNextChangeNotification(dwChangeHandle);
        }
        else if(dwWaitStatus == WAIT_OBJECT_0 + 1)
        {
            break;
        }
    }

    if(log->m_hEven != NULL)
    {
        CloseHandle(log->m_hEven);
    }
    if(dwChangeHandle != INVALID_HANDLE_VALUE)
    {
        FindCloseChangeNotification(dwChangeHandle);
    }

    return 0;
}

/**   @fn     VOID Log::ShutdownLogging()
 *    @brief  日志功能的关闭函数
 *    @param  无.
 *    @return 无
 */
VOID Log::ShutdownLogging()
{
    SetEvent(m_hEven);
    if (m_LogFileHandle != INVALID_HANDLE_VALUE) 
    {
        CloseHandle(m_LogFileHandle);
    }

    DeleteCriticalSection(&m_CriticalSection);

    if(m_hThread != NULL)
    {
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
    }

	SetEvent(m_hDleEvent);
	if (NULL != m_hDleFileThread)
	{
		WaitForSingleObject(m_hDleFileThread, INFINITE);
		CloseHandle(m_hDleFileThread);
	}
}

time_t Log::GetConfigTime()
{
    WIN32_FILE_ATTRIBUTE_DATA attribute;
    memset(&attribute, 0, sizeof(WIN32_FILE_ATTRIBUTE_DATA));
    if (!GetFileAttributesEx(m_ConfigFile, GetFileExInfoStandard, &attribute))
    {
        return time(NULL);
    }

    CTime modifyTime(attribute.ftLastWriteTime);
    return (time_t)modifyTime.GetTime();
}

BOOL Log::ConfigFileIsWrite()
{
    time_t writeTime = GetConfigTime();

    if (m_timeCfg != writeTime)
    {
        m_timeCfg = writeTime;
        return TRUE;
    }

    return FALSE;
}


DWORD WINAPI Log::DleFileFun(LPVOID lParam)
{
	Log* log = (Log*)lParam;

	for(;;)
	{
		if (WaitForSingleObject(log->m_hDleEvent, 10 * 60 * 1000) == WAIT_OBJECT_0)
		{
			break;
		}

		log->GetLogFileInfo();			
	}

	return 0;
}

//排序函数
bool IsFront(const LogfileInfo file1, const LogfileInfo file2)
{
	return file1.tCreateTime < file2.tCreateTime;
}

//获取日志文件链表
DWORD Log::GetLogFileInfo()
{
	time_t fileTime;
	CFileFind cfind;
	CString strFileName;
	CString strFilePath;
	strFileName.Format("%s\\*.LOG", m_szLogDir);
	BOOL bFind = cfind.FindFile(strFileName, 0);
    list<LogfileInfo> listLogfile;

	//循环查找文件
	for (;;)
	{
		bFind = cfind.FindNextFile();

		if (!bFind)
		{
			break;
		}

		//是目录则继续找
		if (cfind.IsDots())
		{
			continue;
		}

		strFilePath = cfind.GetFilePath();
		WIN32_FILE_ATTRIBUTE_DATA struFileData;
		memset(&struFileData, 0, sizeof(struFileData));
		if (!GetFileAttributesEx(strFilePath, GetFileExInfoStandard, &struFileData))
		{
			continue;
		}

		CTime tCreatTime(struFileData.ftCreationTime);
		fileTime = (time_t)tCreatTime.GetTime();
		
		LogfileInfo struFileInfo;
		struFileInfo.strPath = strFilePath;
		struFileInfo.tCreateTime = fileTime;
		listLogfile.push_back(struFileInfo);
	}

	if (listLogfile.size() <= 1)
	{
		return 0;
	}

	//排序
	listLogfile.sort(IsFront);

	//限制日志文件个数2~50个
	if (m_LogFileMaxNum < 2)
	{
		m_LogFileMaxNum = 2;
	}
	if (m_LogFileMaxNum > 50)
	{
		m_LogFileMaxNum = 50;
	}

	//循环删除文件
	LogfileInfo struFileInfo;
	int iNeedDlefileNum = listLogfile.size() - m_LogFileMaxNum;
	while ((iNeedDlefileNum--) >= 0)
	{
		struFileInfo = listLogfile.front();
		listLogfile.pop_front();
		//删除两次
		if (!DeleteFile(struFileInfo.strPath))
		{
			DeleteFile(struFileInfo.strPath);
		}		
	}

	return 1;
}