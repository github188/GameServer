
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
#define LOG(fm, ...)      Log::GetInstance()->LogSpew(fm,__VA_ARGS__) ///�����õļ�¼��Ϣ����
#else
#define LOG		Log::GetInstance()->LogSpew
#endif

struct LogfileInfo
{
	CString strPath;
	time_t tCreateTime;
};

/**   @class Log [log.cpp]
 *    @brief �ṩ��־ģ�����ع��ܡ�
 */
class Log
{
public:
	Log();

    ~Log();
private:
    static Log*      m_LogInstance;                ///��־��ĵ���ʵ��
    char             m_ConfigFile[MAX_PATH];       ///�����ļ�·��
    DWORD            m_LogFlags;                   ///����ѡ��ı�ʾ
    char             m_szLogFileName[MAX_PATH+1];  ///��־�ļ���
    HANDLE           m_LogFileHandle;              ///��־�ļ����
    DWORD            m_LogVMLevel;                 ///������Ϣ��������Ƽ���
    HANDLE           m_hThread;                    ///����߳̾��
	HANDLE           m_hDleFileThread;             ///ɾ����־�ļ��߳̾��
	HANDLE           m_hDleEvent;                  ///ɾ����־�ļ��¼����
    HANDLE           m_hEven;                      ///�������߳̽������¼����
    DWORD            m_LogFileSize;                ///������־�ļ����������
	DWORD            m_LogFileMaxNum;           ///ά������־�ļ�����
    CRITICAL_SECTION m_CriticalSection;            ///�ٽ������
    
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
