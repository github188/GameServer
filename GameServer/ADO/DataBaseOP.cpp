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
Desc:		�����ݿ��л�ȡ����ֵ
Input:	    recordSet: ��¼��
rowName: ����
Output:	iValue
Return:	 ����ֵ
note:		����¼������VT_NULLʱ��ֵΪ-1
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
Desc:		�����ݿ��л�ȡ����ֵ
Input:	    recordSet: ��¼��
rowName: ����
Output:	strValue
Return:	    ��
note:			����¼������VT_NULLʱ��ֵΪ��
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
Desc:		�����ݿ��л�ȡ����ֵ
Input:	    recordSet: ��¼��
rowName: ����
Output:	strValue
Return:	    ��
note:			����¼������VT_NULLʱ��ֵΪ��
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
Desc:		�����ݿ��л�ȡ���ڼ�ʱ��ֵ
Input:	    recordSet: ��¼��
rowName:    ����
Output:	    strValue
Return:	    ��
note:		����¼������VT_NULLʱ��ֵΪ��
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
Description: �������е���һ��ֵ
Input:		 strSequenceName����������
Output:      void
Return:      -1ʧ�ܣ������ɹ���������ֵ
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
Desc:		�����ݿ��л�ȡʱ��ֵ
Input:	    recordSet: ��¼��
rowName: ����
Output:	strValue
Return:	    ��
note:			����¼������VT_NULLʱ��ֵΪ��
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
Desc:		�����ݿ��л�ȡʱ��ֵ
Input:	    recordSet: ��¼��
rowName: ����
Output:	ctDateTime:����ʱ�����
Return:	    ��
note:			����¼������VT_NULLʱ��ֵΪ��
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
Desc:		���SQL����ܷ�鵽��Ϣ
Input:	    strSql:sql���
Output:	��
Return:	����Ϣ:TRUE, ����Ϣ:FALSE	
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
Desc:		��ȡ���Ԫ����Ŀ
Input:	    tableName:����
Output:	��
Return:	    Ԫ����Ŀ
note:			������ʱ����0
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
Desc:		��ȡsql����ѯ�Ľ����Ŀ
Input:	    strSql:sql���
Output:	��
Return:	Ԫ����Ŀ
note:		�򿪼�¼��ʧ��ʱ����0
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
Desc:		��ȡ��ѯ��Ŀsql���ķ��ؽ��
Input:	    strSql: sql���
				rowName: ����
Output:	��
Return:	�����Ŀ
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