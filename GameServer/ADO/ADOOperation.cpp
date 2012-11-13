#include "stdafx.h"
#include "AdoOperation.h"

CADOOperation::CADOOperation()
{
	m_strConnection = "";
	m_lConnectionOpenType = 0;
    CoInitialize(NULL);
}

CADOOperation::~CADOOperation()
{
    Stop();
    CoUninitialize();
}

int CADOOperation::GetConnectionState()
{
    if(m_pConnection)
    {
        if(m_pConnection->State)	
        {
            return 1;
        }
        else	
        {
            return 0;
        }
    }
    else	
    {
        return -1;
    }
}

BOOL CADOOperation::CreateInstance()
{
    CAutoLock Auto(m_ExecuteSqlLock);
	if(m_pConnection == NULL)
	{
		//线程中重建时将会出错
		//HRESULT hr = m_pConnection.CreateInstance(__uuidof(Connection));///创建Connection对象
		HRESULT hr = m_pConnection.CreateInstance("ADODB.Connection");///创建Connection对象
		if(!SUCCEEDED(hr))
		{
			return FALSE;
		}
	}
	if(m_pRecordset == NULL)
	{
		//HRESULT hr = m_pRecordset.CreateInstance(__uuidof(Recordset));///创建Connection对象
		HRESULT hr = m_pRecordset.CreateInstance("ADODB.Recordset");///创建Connection对象
		if(!SUCCEEDED(hr))
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CADOOperation::OpenConnection(CString strConnection, long lOpenType)
{
	CAutoLock Auto(m_ExecuteSqlLock);
	COleDateTime odtNow = COleDateTime::GetCurrentTime();
    HRESULT hr = NULL;
	try
	{
		if(m_pConnection == NULL)
		{
			return FALSE;
		}
		if(GetConnectionState() != 1)
		{
			m_strConnection = strConnection;
			m_lConnectionOpenType = lOpenType;
			
			//UDL文件方式
			m_pConnection->ConnectionTimeout = 10;	//10秒超时，虽然不太准
			switch(lOpenType)
			{
			case ADO_UDL_FILE_TYPE:
				m_pConnection->ConnectionString = (_bstr_t)strConnection;
				hr = m_pConnection->Open("", "", "", adConnectUnspecified);
				if(!SUCCEEDED(hr))
				{
					CloseConnection();
					return FALSE;
				}
				break;
			case ADO_FIRST_PARAMETERS_TYPE:
				hr = m_pConnection->Open((_bstr_t)strConnection, "", "", adConnectUnspecified);
				if(!SUCCEEDED(hr))
				{
					CloseConnection();
					return FALSE;
				}
				break;
			default:
				;
			}
			if(!SUCCEEDED(hr))
			{
				CloseConnection();
				return FALSE;
			}
		}
	}
	catch(_com_error e)///捕捉异常
	{
		
		CloseConnection();
		CString errormessage;
		errormessage.Format(_T("打开连接时异常，错误信息:%d"),e.WCode());
		OutputDebugString(errormessage);
		return FALSE;
	}
	return TRUE;//连接数据库
}

void CADOOperation::CloseConnection()
{
	//关闭记录集，关闭连接
	if(m_pRecordset)
	{
		if(m_pRecordset->State)
			m_pRecordset->Close();
	}
	if(m_pConnection)
	{
		if(m_pConnection->State)
			m_pConnection->Close(); ///如果已经打开了连接则关闭它
	}
}

void CADOOperation::ReleaseInstance()
{
	CAutoLock Auto(m_ExecuteSqlLock);
	//关闭连接
	CloseConnection();
	//没有以下几句也可以
	if(m_pRecordset)
	{
		m_pRecordset.Release();
		m_pRecordset = NULL;
	}
	if(m_pConnection)
	{
		m_pConnection.Release();
		m_pConnection = NULL;
	}
}

//利用事务执行多条SQL语句，第二个参数为执行语句条数
BOOL CADOOperation::ExecuteMulSQL(CString strSql[], int iNum)
{
	CAutoLock Auto(m_ExecuteSqlLock);
	if(GetConnectionState() != 1)
	{				
		PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd,WM_DBSTATE,1,NULL);		
	}

	CString strInfo;
	CString strTrace;	
	bool bRet = false;
	if (GetConnectionState() == 1)
	{
		try
		{ 
			m_pConnection->BeginTrans();
			for (int i = 0; i < iNum; i++)
			{
				strInfo.Format("%s", strSql[i]);			
				m_pConnection->Execute((_bstr_t)strInfo, NULL, adCmdText);	
				TRACE("==YUQIANG== ExecuteMulSQL ------      iNum: %d,  sql: %s", i, strSql[i]);
			}
			bRet = true;
			m_pConnection->CommitTrans();
		}
		catch(_com_error e)
		{
			CString strError = _T("");
			m_pConnection->RollbackTrans();
			strError.Format("ExecuteSql %s %s", "数据库操作失败", e.ErrorMessage()); 
			OutputDebugString(strError);
			bRet = false;
		}
	}

	return bRet;
}


BOOL CADOOperation::ExecuteSQL(CString CommandText, long Options)
{
	CAutoLock Auto(m_ExecuteSqlLock);
	if(GetConnectionState() == 1)
	{
		COleDateTime odtNow = COleDateTime::GetCurrentTime();
		try
		{
			_variant_t RecordsAffected; //VARIANT类型不行
			switch(Options)
			{
			case adCmdText:
				m_pConnection->Execute((_bstr_t)CommandText, &RecordsAffected, adCmdText);
				RecordsAffected.Clear();
				break;
			case adCmdStoredProc:
				m_pConnection->Execute((_bstr_t)CommandText, &RecordsAffected, adCmdStoredProc);
				RecordsAffected.Clear();
				break;
			default:	//未知类型语句
				break;
			}
		}
		catch(_com_error e)///捕捉异常
		{
			CString errormessage;
			errormessage.Format(_T("执行SQL语句时失败，连接关闭！错误信息:%s"),e.ErrorMessage());

			//错误的话有不关闭连接
			DWORD dwError = e.WCode();
			if(3079 != dwError && 3421 != dwError)
			{
				CloseConnection();
				PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd,WM_DBSTATE,1,NULL);
			}
			OutputDebugString(errormessage + CommandText);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd,WM_DBSTATE,1,NULL);
		return FALSE;
	}
}

BOOL CADOOperation::OpenConnect(void)
{
    return Init(m_strDBServerIP, m_strDBName, m_strDBUser, m_strDBPwd);
}

BOOL CADOOperation::CloseConnect(void)
{
    Stop();
    return TRUE;
}


BOOL CADOOperation::Init(CString strDBServerIP, CString strDBName, CString strDBUser, CString strDBPwd)
{
    m_strDBServerIP = strDBServerIP;
    m_strDBName = strDBName;
    m_strDBUser = strDBUser;
    m_strDBPwd = strDBPwd;

	BOOL bRet = FALSE;
	ReleaseInstance();
	if(CreateInstance())
	{
		CString strConnection;
        strConnection.Format(_T("Provider=SQLOLEDB.1;Persist Security Info=True;User ID=%s;Password=%s;Initial Catalog=%s;Data Source=%s")
            , strDBUser, strDBPwd, strDBName, strDBServerIP);
		if(OpenConnection(strConnection, ADO_FIRST_PARAMETERS_TYPE))
		{
			bRet = TRUE;
		}
		else
		{
            LOG(LEVEL_ERROR, "数据库连接失败%s", strConnection);
		}
	}
	return bRet;
}

BOOL CADOOperation::Stop()
{
	ReleaseInstance();
	return TRUE;
}


const _RecordsetPtr CADOOperation::OpenRecordset(CString CommandText)
{
    CAutoLock Auto(m_ExecuteSqlLock);
	if (1 != GetConnectionState())
	{
		if (!OpenConnect())
		{
			return NULL;
		}
	}

	COleDateTime odtNow = COleDateTime::GetCurrentTime();
	_variant_t bstrSQL = (LPCTSTR)CommandText;
	try
	{
		HRESULT hr = m_pRecordset->Open(bstrSQL, _variant_t((IDispatch*)m_pConnection,TRUE), adOpenForwardOnly, adLockReadOnly, adCmdText);
		bstrSQL.Clear();

		if(!SUCCEEDED(hr))
		{
			return NULL;
		}
		return m_pRecordset;
	}
	catch(_com_error e)///捕捉异常
	{  
		bstrSQL.Clear();
		CString errormessage;
		errormessage.Format(_T("打开记录集时失败!错误信息:%s"),e.ErrorMessage());

		long errorcount = m_pConnection->GetErrors()->GetCount();
		_bstr_t add;
		CString ErrorMessage,temp;
		for(short i=0; i<errorcount; i++)
		{
			add = m_pConnection->GetErrors()->GetItem(_variant_t((short)i))->GetDescription();
			temp = (char *)add;
			ErrorMessage += temp;
		}
		//错误的话有不关闭连接
		DWORD dwError = e.WCode();
		if(3079 != dwError && 3421 != dwError && 3265 != dwError)
		{
			CloseConnection();
		}
		OutputDebugString(ErrorMessage + CommandText);
		//失败后关连接
		CloseConnect();
		return NULL;
	}
	return m_pRecordset;
}
const _RecordsetPtr CADOOperation::OpenRecordsetEx(CString CommandText)
{
	CAutoLock Auto(m_ExecuteSqlLock);
	if(GetConnectionState() != 1)
	{
		//OpenConnection(m_strConnection, m_lConnectionOpenType);
		PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd,WM_DBSTATE,1,NULL);
	}
	if(GetConnectionState() == 1)
	{
		COleDateTime odtNow = COleDateTime::GetCurrentTime();
		_variant_t bstrSQL = (LPCTSTR)CommandText;
		try
		{

			HRESULT hr = m_pRecordset->Open(bstrSQL, _variant_t((IDispatch*)m_pConnection,TRUE), adOpenKeyset, adLockOptimistic, adCmdText);
			bstrSQL.Clear();

			if(!SUCCEEDED(hr))
			{
				return NULL;
			}
			return m_pRecordset;
		}
		catch(_com_error e)///捕捉异常
		{  
			bstrSQL.Clear();
			CString errormessage;
			errormessage.Format(_T("打开记录集时失败！错误信息:%s"),e.ErrorMessage());

			long errorcount = m_pConnection->GetErrors()->GetCount();
			_bstr_t add;
			CString ErrorMessage,temp;
			for(short i=0; i<errorcount; i++)
			{
				add = m_pConnection->GetErrors()->GetItem(_variant_t((short)i))->GetDescription();
				temp = (char *)add;
				ErrorMessage += temp;
			}
			//错误的话有不关闭连接
			DWORD dwError = e.WCode();
			if(3079 != dwError && 3421 != dwError && 3265 != dwError)
			{
				CloseConnection();
				PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd,WM_DBSTATE,1,NULL);
			}
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

BOOL CADOOperation::OpenRecordsetEx(_RecordsetPtr& record, const CString& strSql)
{
	CAutoLock Auto(m_ExecuteSqlLock);
	//创建记录集实例失败
	if (!CreateRecordset(record))
	{
		return FALSE;
	}

	//数据库连接断开
	if(GetConnectionState() != 1)
	{
		PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd,WM_DBSTATE,1,NULL);
		return FALSE;
	}

	HRESULT hr = NULL;
	BOOL bSucceed = FALSE;
	try
	{
	    hr = record->Open((_bstr_t)strSql, _variant_t((IDispatch*)m_pConnection,TRUE), adOpenForwardOnly, adLockReadOnly, adCmdText);
		bSucceed = SUCCEEDED(hr);
	}
	catch(_com_error& e)
	{
		CString errormessage;
		errormessage.Format(_T("打开记录集时失败！错误信息:%s"),e.ErrorMessage());
		OutputDebugString(errormessage);

		bSucceed = FALSE;
	}

	if (!bSucceed && NULL != record && adStateOpen == record->State)
	{
		record->Close();
	}

	return bSucceed;
}

BOOL CADOOperation::CreateRecordset(_RecordsetPtr& record)
{
	HRESULT hr = record.CreateInstance(_uuidof(Recordset));
	return (SUCCEEDED(hr));
}

/**	@fn	void CADOOperation::CloseRecordset()
*	@brief	关闭记录集，添加了对打开状态的判断
*	@param
*			无
*	@return
*			- 返回类型：int
*			- 返回说明：
*				- 返回相应的错误码
*/
void CADOOperation::CloseRecordset()
{
	if(NULL != m_pRecordset)
	{
		if(m_pRecordset->State != adStateClosed)
		{
			m_pRecordset->Close();
			m_pRecordset = NULL;
		}
	}
}

BOOL CADOOperation::RecordClassLog( int iUserId, int iUserType, int iClassId, int iType )
{

    if (iUserId <= 0 || iClassId <= 0)
    {
        return FALSE;
    }

    CString strSQL = "";
    strSQL.Format("INSERT INTO Wz_CourseLog(CourseId, UserId, Time, LType, UserType)\
                  VALUES (%d, %d, GetDate(), %d, %d);", iClassId, iUserId, iType, iUserType);

    return ExecuteSQL(strSQL);
}

