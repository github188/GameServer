#pragma once

//定义一些数据库的简便操作
//不适用于多线程环境
class CDataBaseOP
{
public:
	CDataBaseOP();
	~CDataBaseOP();
	static void GetDBString(const _RecordsetPtr& recordSet, const CString& rowName, CString& strValue);
	static int GetDBInt(const _RecordsetPtr& recordSet, const CString& rowName, int& iValue);
	static void GetDBDate(const _RecordsetPtr& recordSet, const CString& rowName, CString& strValue);//获取日期
	static void GetDBDateTime(const _RecordsetPtr& recordSet, const CString& rowName, CString& strValue); //获取日期及时间
	static void GetDBTime(const _RecordsetPtr& recordSet, const CString& rowName, COleDateTime& ctDateTime);
	static int GetSequenceNextValue(const CString& strSequenceName);
	static void GetDBTime(const _RecordsetPtr& recordSet, const CString& rowName, CString& strValue);
	static BOOL CheckInfoExsist(const CString& strSql);
	static int GetTableRowNum(const CString& tableName);
	static int GetInfoCount(const CString& strSql);
	static int GetQueryCount(const CString& strSql, const CString& rowName);
};