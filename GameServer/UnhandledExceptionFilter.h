#pragma once
#include <fstream>
#include "BugslayerUtil.h"
#pragma comment(lib, "bugslayerutil.lib")

class CUnhandledExceptionFilter
{
public:
	//构造函数
	CUnhandledExceptionFilter()
	{
		TCHAR szPath[MAX_PATH];
		GetModuleFileName(NULL, szPath, MAX_PATH);

		CString csPath(szPath);
		FileName() = csPath.Left(csPath.ReverseFind(_T('.'))) + _T("_exp.log"); 

		std::ofstream fout(FileName(), std::ios_base::out | std::ios_base::app);
		fout.seekp(std::ios_base::end);
		fout<<CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S ")) << _T("Start")<<std::endl;
		fout.close();

		//设置异常处理过滤器
		SetCrashHandlerFilter(_UnhandledExceptionFilter);
	}

	//析构函数
	~CUnhandledExceptionFilter()
	{
		std::ofstream fout(FileName(), std::ios_base::out | std::ios_base::app);
		fout.seekp(std::ios_base::end);
		fout<<CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S ")) << _T("End")<<std::endl;
		fout.close();
	}

	//获取异常的堆栈信息
	static CString GetCrashInfoString(PEXCEPTION_POINTERS pExPtrs)
	{
		CString csExceptLogs, csTemp;
		
		//获取出现异常的原因
		csExceptLogs = csExceptLogs + GetFaultReason ( pExPtrs ) + _T("\r\n\r\n") ;
		
		//获取出现异常时CPU寄存器状态信息
		csExceptLogs = csExceptLogs + GetRegisterString ( pExPtrs ) + _T("\r\n\r\n") ;
		

		//枚举调用堆栈
		const char * szBuff = GetFirstStackTraceString ( GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE, pExPtrs  ) ;
		do
		{
			csExceptLogs = csExceptLogs + szBuff + _T("\r\n");
			szBuff = GetNextStackTraceString ( GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE , pExPtrs ) ;
		}
		while ( NULL != szBuff ) ;//end while( NULL != szBuff )

		return csExceptLogs;
	}
private:

	//异常处理过滤器
	static LONG __stdcall _UnhandledExceptionFilter(PEXCEPTION_POINTERS pExPtrs)
	{
		std::ofstream fout(FileName(), std::ios_base::out | std::ios_base::app);
		fout.seekp(std::ios_base::end);
		fout<<CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S "))<<(LPCTSTR)GetCrashInfoString(pExPtrs)<<std::endl;
		fout.close();

//		ShellExecute(NULL, "open", szPath, NULL, NULL, NULL);
		
		return EXCEPTION_EXECUTE_HANDLER;
	}

	//获取程序执行的绝对路径
	static CString& FileName()
	{
		static CString m_csFile;
		return m_csFile;
	}
	bool m_bRestart;
};
