#pragma once


struct STRU_REGIONINFO
{
    int iRegionID;//ID
    char chRegionName[62];//����
    int iParentID;//�ϼ�����
    int iRegionLevel;//������

    STRU_REGIONINFO()
    {
        iRegionID = -1;
        memset(chRegionName, 0, sizeof(chRegionName));
        iParentID = -1;
        iRegionLevel = -1;
    }
};

#define WM_DBSTATE WM_USER + 133 //���ݿ�����
enum EADO_CONNECTION_OPEN_TYPE
{
	ADO_UDL_FILE_TYPE = 0,
	ADO_FIRST_PARAMETERS_TYPE = 1,
};

class CADOOperation  
{
public:
	CADOOperation();
	virtual ~CADOOperation();

	BOOL CreateInstance();
	void ReleaseInstance();
	BOOL OpenConnection(CString strConnection, long lOpenType);//�������ݿ�
	void CloseConnection();
	int GetConnectionState();
	BOOL ExecuteSQL(CString CommandText, long Options = adCmdText);
	BOOL ExecuteMulSQL(CString strSql[], int iNum);
	const _RecordsetPtr	OpenRecordset(CString CommandText);//ֻ���򿪼�¼��
	const _RecordsetPtr OpenRecordsetEx(CString CommandText);//�򿪼�¼��
	void CloseRecordset(); //�رռ�¼��������˶Դ�״̬���ж�
	BOOL OpenRecordsetEx(_RecordsetPtr& record, const CString& strSql);

	_ConnectionPtr m_pConnection;
	_RecordsetPtr m_pRecordset;
	CString m_strConnection;
	long m_lConnectionOpenType;

    CString m_strDBServerIP;
    CString m_strDBName;
    CString m_strDBUser;
    CString m_strDBPwd;

    BOOL Init(CString strDBServerIP, CString strDBName, CString strDBUser, CString strDBPwd);
    BOOL Stop();

    BOOL OpenConnect(void);
    BOOL CloseConnect(void);
private:
    CLock m_ExecuteSqlLock;
	BOOL CreateRecordset(_RecordsetPtr& record);

public:
    BOOL RecordClassLog(int iUserId, int iUserType, int iClassId, int iType);
};