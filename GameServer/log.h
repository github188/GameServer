
#ifndef __LOG_H__
#define __LOG_H__

#define CONFIG_FILE "logcfg.ini" 
#define DEFAULT_LOGFILE_NAME    "log.LOG" 
#define CONFIG_FILE_SIZE 10240 

#define LEVEL_INFO        4      
#define LEVEL_WARNING     3
#define LEVEL_ERROR       2
#define LEVEL_FATALERROR  1
#define LEVEL_OUTPUT      1


#ifdef  MAX_PATH
#undef  MAX_PATH
#define MAX_PATH                (1024*8)
#endif

#define LOG_ENABLE_FILE_LOGGING         0x0001     
#define LOG_ENABLE_DEBUGGER_LOGGING     0x0020
#define LOG_ENABLE                      0x0040

#if _MSC_VER > 1400
#define LOG(fm, ...)      Log::GetInstance()->LogSpew(fm,__VA_ARGS__) ///供调用的记录信息函数
#else
#define LOG		Log::GetInstance()->LogSpew
#endif

struct LogfileInfo
{
	CString strPath;
	time_t tCreateTime;
};

/**   @class Log [log.cpp]
 *    @brief 提供日志模块的相关功能。
 */
class Log
{
public:
	Log();

    ~Log();
private:
    static Log*      m_LogInstance;                ///日志类的单例实例
    char             m_ConfigFile[MAX_PATH];       ///配置文件路径
    DWORD            m_LogFlags;                   ///配置选项的标示
    char             m_szLogFileName[MAX_PATH+1];  ///日志文件名
    HANDLE           m_LogFileHandle;              ///日志文件句柄
    DWORD            m_LogVMLevel;                 ///输入信息的最高限制级别
    HANDLE           m_hThread;                    ///监控线程句柄
	HANDLE           m_hDleFileThread;             ///删除日志文件线程句柄
	HANDLE           m_hDleEvent;                  ///删除日志文件事件句柄
    HANDLE           m_hEven;                      ///控制子线程结束的事件句柄
    DWORD            m_LogFileSize;                ///单个日志文件的最大容量
	DWORD            m_LogFileMaxNum;           ///维护的日志文件个数
    CRITICAL_SECTION m_CriticalSection;            ///临界区句柄
    
    VOID InitLogging();
    VOID LogSpewValist(DWORD level, const char *fmt, va_list args);
	VOID LoadConfig();
    bool LoggingOn(DWORD level);
    static DWORD WINAPI Monitor(LPVOID lpParameter);
	static DWORD WINAPI DleFileFun(LPVOID lParam);
    void LocateConfigFile();
    BOOL ConfigFileIsWrite();
    time_t GetConfigTime();
	DWORD GetLogFileInfo();
    DWORD GetConfigDWORD(char* FilePath, char* LogFlag, DWORD flag = 0xffffffff);

    TCHAR m_szLogDir[MAX_PATH];
    time_t m_timeCfg;

public:
    static Log* GetInstance();
    VOID ShutdownLogging();
    VOID LogSpew(DWORD level, const char *fmt, ... );
};

#endif //__LOG_H__
