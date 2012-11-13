#pragma once
#include <fstream>
#include "BugslayerUtil.h"
#pragma comment(lib, "bugslayerutil.lib")

class CUnhandledExceptionFilter
{
public:
	//���캯��
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

		//�����쳣���������
		SetCrashHandlerFilter(_UnhandledExceptionFilter);
	}

	//��������
	~CUnhandledExceptionFilter()
	{
		std::ofstream fout(FileName(), std::ios_base::out | std::ios_base::app);
		fout.seekp(std::ios_base::end);
		fout<<CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S ")) << _T("End")<<std::endl;
		fout.close();
	}

	//��ȡ�쳣�Ķ�ջ��Ϣ
	static CString GetCrashInfoString(PEXCEPTION_POINTERS pExPtrs)
	{
		CString csExceptLogs, csTemp;
		
		//��ȡ�����쳣��ԭ��
		csExceptLogs = csExceptLogs + GetFaultReason ( pExPtrs ) + _T("\r\n\r\n") ;
		
		//��ȡ�����쳣ʱCPU�Ĵ���״̬��Ϣ
		csExceptLogs = csExceptLogs + GetRegisterString ( pExPtrs ) + _T("\r\n\r\n") ;
		

		//ö�ٵ��ö�ջ
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

	//�쳣���������
	static LONG __stdcall _UnhandledExceptionFilter(PEXCEPTION_POINTERS pExPtrs)
	{
		std::ofstream fout(FileName(), std::ios_base::out | std::ios_base::app);
		fout.seekp(std::ios_base::end);
		fout<<CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S "))<<(LPCTSTR)GetCrashInfoString(pExPtrs)<<std::endl;
		fout.close();

//		ShellExecute(NULL, "open", szPath, NULL, NULL, NULL);
		
		return EXCEPTION_EXECUTE_HANDLER;
	}

	//��ȡ����ִ�еľ���·��
	static CString& FileName()
	{
		static CString m_csFile;
		return m_csFile;
	}
	bool m_bRestart;
};
