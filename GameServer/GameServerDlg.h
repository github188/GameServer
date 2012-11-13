// GameServerDlg.h : ͷ�ļ�
//

#pragma once
#include "FlashSecurity.h"
#include "iocp_server.hpp"
#include "afxcmn.h"
#include "WatchDog.h"

// CGameServerDlg �Ի���
class CGameServerDlg : public CDialog
{
// ����
public:
	CGameServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_GAMESERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT OnGISInstance(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();

private:
    void Init();
    static DWORD WINAPI ServerThread(LPVOID lParam);
    void StartServer(void);

    static DWORD WINAPI CheckThread(LPVOID lParam);
    void CheckProc(void);
    HANDLE m_hExitEvent;

    void GetMemoryInfo();
    bool InitDB();

private:
    CFlashSecurity  m_FlashSecurity;
    GameServer::iocp_server m_iocp;
    HANDLE m_hServerThread;
    NOTIFYICONDATA  m_NoitfyIcon;
    vector<CString> m_LogVector;
    CLock           m_LogVectorLock;
    CTime           m_StartTime;
    unsigned short  m_sPort;
    WatchDog        m_WatchDog;

    CString m_strDBServerIP;
    CString m_strDBName;
    CString m_strDBUser;
    CString m_strDBPwd;

public:
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    void ShowLog(LPCTSTR ptzFormat, ...);
    CListCtrl m_LogList;
    afx_msg void OnNMRClickListLog(NMHDR *pNMHDR, LRESULT *pResult);
    void GetVersionInfo(void);

    CString m_strConfigPath;
    CString GetConfigPath();
    void LoadConfig();
};

extern CGameServerDlg* g_pDlg;