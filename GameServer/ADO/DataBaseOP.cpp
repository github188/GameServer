#include "StdAfx.h"
#include "databaseop.h"

CDataBaseOP::CDataBaseOP(void)
{

}

CDataBaseOP::~CDataBaseOP(void)
{
}

/*********************************************************
Function:	GetDBInt
Desc:		从数据库中获取整形值
Input:	    recordSet: 记录集
rowName: 列名
Output:	iValue
Return:	 整型值
note:		当记录集返回VT_NULL时赋值为-1
**********************************************************/
int CDataBaseOP::GetDBInt(const _RecordsetPtr& recordSet, const CString& rowName, int& iValue)
{
	_variant_t var;
	var = recordSet->GetCollect((_bstr_t)rowName);
	if (VT_NULL != var.vt && VT_EMPTY != var.vt)
	{
		iValue = (int)var;
		var.Clear();
		return iValue;
	}
	iValue = -1;
	return iValue;
}

/*********************************************************
Function:	GetDBString
Desc:		从数据库中获取整形值
Input:	    recordSet: 记录集
rowName: 列名
Output:	strValue
Return:	    无
note:			当记录集返回VT_NULL时赋值为空
**********************************************************/
void CDataBaseOP::GetDBString(const _RecordsetPtr& recordSet, const CString& rowName, CString& strValue)
{
	_variant_t var;
	var = recordSet->GetCollect((_bstr_t)rowName);
	if (VT_NULL != var.vt)
	{
		strValue = (LPCSTR)_bstr_t(var);
		strValue.Trim();
		var.Clear();
		return;
	}
	strValue = _T("");
}

/*********************************************************
Function:	GetDBDate
Desc:		从数据库中获取日期值
Input:	    recordSet: 记录集
rowName: 列名
Output:	strValue
Return:	    无
note:			当记录集返回VT_NULL时赋值为空
**********************************************************/
void CDataBaseOP::GetDBDate(const _RecordsetPtr& recordSet, const CString& rowName, CString& strValue)
{
	_variant_t var;
	var = recordSet->GetCollect((_bstr_t)rowName);
	if (VT_NULL != var.vt)
	{
		strValue = ((COleDateTime)var.date).Format("%Y-%m-%d");
		strValue.Trim();
		var.Clear();
		return;
	}
	strValue = _T("");
}

/*********************************************************
Function:	GetDBDate
Desc:		从数据库中获取日期及时间值
Input:	    recordSet: 记录集
rowName:    列名
Output:	    strValue
Return:	    无
note:		当记录集返回VT_NULL时赋值为空
**********************************************************/
void CDataBaseOP::GetDBDateTime(const _RecordsetPtr& recordSet, const CString& rowName, CString& strValue)
{
	_variant_t var;
	var = recordSet->GetCollect((_bstr_t)rowName);
	if (VT_NULL != var.vt)
	{ 
		strValue = ((COleDateTime)var.date).Format("%Y-%m-%d %H:%M:%S");
		var.Clear();
		return;
	}
}

/*************************************************
Function:    GetSequenceNextValue
Description: 返回序列的下一个值
Input:		 strSequenceName：序列名称
Output:      void
Return:      -1失败，，，成功返回序列值
*************************************************/
int CDataBaseOP::GetSequenceNextValue(const CString& strSequenceName)
{
	_RecordsetPtr recordSet;
	int iNextValue = -1;
	CString strSql;

	strSql.Format("select %s.nextval as NextID from dual", strSequenceName);
	if (!theApp.m_pADOOperation->OpenRecordsetEx(recordSet, strSql))
	{
		return -1;
	}

	if (NULL != recordSet)
	{
		if(!recordSet->adoEOF)
		{
			CDataBaseOP::GetDBInt(recordSet, "NextID", iNextValue);
		}
		recordSet->Close();
	}
	return iNextValue;
}

/*********************************************************
Function:	GetDBTime
Desc:		从数据库中获取时间值
Input:	    recordSet: 记录集
rowName: 列名
Output:	strValue
Return:	    无
note:			当记录集返回VT_NULL时赋值为空
**********************************************************/
void CDataBaseOP::GetDBTime(const _RecordsetPtr& recordSet, const CString& rowName, CString& strValue)
{
	_variant_t var;
	var = recordSet->GetCollect((_bstr_t)rowName);
	COleDateTime time;
	if (VT_NULL != var.vt)
	{
		time = (COleDateTime)var.date;
		strValue = time.Format("%Y-%m-%d %H:%M:%S");
		strValue.Trim();
		var.Clear();
		return;
	}
	strValue = _T("");
}

/*********************************************************
Function:	GetDBTime
Desc:		从数据库中获取时间值
Input:	    recordSet: 记录集
rowName: 列名
Output:	ctDateTime:日期时间对象
Return:	    无
note:			当记录集返回VT_NULL时赋值为空
**********************************************************/
void CDataBaseOP::GetDBTime(const _RecordsetPtr& recordSet, const CString& rowName,  COleDateTime& ctDateTime)
{
	_variant_t var;
	var = recordSet->GetCollect((_bstr_t)rowName);
	if (VT_NULL != var.vt)
	{
		ctDateTime = ((COleDateTime)var.date);
		var.Clear();
		return;
	}
}

/*********************************************************
Function:	CheckInfoExsist
Desc:		检测SQL语句能否查到信息
Input:	    strSql:sql语句
Output:	无
Return:	有信息:TRUE, 无信息:FALSE	
**********************************************************/
BOOL CDataBaseOP::CheckInfoExsist(const CString& strSql)
{
	_RecordsetPtr recordSet = theApp.m_pADOOperation->OpenRecordset(strSql);
	if (NULL == recordSet)
	{
		return FALSE;
	}

	int iCount = 0;
	while(!recordSet->adoEOF)
	{
		iCount++;
		recordSet->MoveNext();
	}
	recordSet->Close();
	return (iCount > 0) ? TRUE : FALSE;
}

/*********************************************************
Function:	GetTableRowNum
Desc:		获取表的元素数目
Input:	    tableName:表名
Output:	无
Return:	    元素数目
note:			表不存在时返回0
**********************************************************/
int CDataBaseOP::GetTableRowNum(const CString& tableName)
{
	CString strSql;
	int nRowNum = 0;

	strSql.Format(_T("SELECT COUNT(*) AS IROWNUM FROM %s"),tableName);
	_RecordsetPtr recordSet = theApp.m_pADOOperation->OpenRecordset(strSql);

	if (NULL == recordSet)
	{
		return 0;
	}

	if (VARIANT_FALSE == recordSet->adoEOF)
	{
		CDataBaseOP::GetDBInt(recordSet, "IROWNUM", nRowNum);
	}

	recordSet->Close();
	return nRowNum;
}

/*********************************************************
Function:	GetInfoCount
Desc:		获取sql语句查询的结果数目
Input:	    strSql:sql语句
Output:	无
Return:	元素数目
note:		打开记录集失败时返回0
**********************************************************/
int CDataBaseOP::GetInfoCount(const CString& strSql)
{
	_RecordsetPtr recordSet = theApp.m_pADOOperation->OpenRecordset(strSql);
	if (NULL == recordSet)
	{
		return 0;
	}

	int iCount = 0;
	while(!recordSet->adoEOF)
	{
		iCount++;
		recordSet->MoveNext();
	}
	recordSet->Close();
	return iCount;
}


/*********************************************************
Function:	GetQueryCount
Desc:		获取查询数目sql语句的返回结果
Input:	    strSql: sql语句
				rowName: 列名
Output:	无
Return:	结果数目
**********************************************************/
int CDataBaseOP::GetQueryCount(const CString& strSql, const CString& rowName)
{
	_RecordsetPtr recordSet;
	if (!theApp.m_pADOOperation->OpenRecordsetEx(recordSet, strSql))
	{
		return 0;
	}
	if (recordSet->adoEOF)
	{
		recordSet->Close();
		return 0;
	}

	int iCount = 0;
	CDataBaseOP::GetDBInt(recordSet, rowName, iCount);
	recordSet->Close();

	return iCount;
}