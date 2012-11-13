#include "StdAfx.h"
#include "FlashSecurity.h"

CFlashSecurity::CFlashSecurity(void)
: m_ListenSock(INVALID_SOCKET)
, m_hWorkThread(NULL)
{
    strcpy_s(m_cXML, 255, "<?xml version=\"1.0\"?>"
        "<cross-domain-policy>"  
        "<site-control permitted-cross-domain-policies=\"all\"/>"
        "<allow-access-from domain=\"*\" to-ports=\"*\"/>"
        "</cross-domain-policy>");
}

CFlashSecurity::~CFlashSecurity(void)
{
}

BOOL CFlashSecurity::Start( void )
{
    WSADATA lpWSAData;
    WSAStartup(MAKEWORD(2, 2), &lpWSAData);
    
    m_hWorkThread = CreateThread(NULL, 0, WorkThread, this, 0, NULL);
    if(NULL == m_hWorkThread)
    {
        LOG(LEVEL_ERROR, "启动安全沙箱服务主线程失败");
        return FALSE;
    }
    LOG(LEVEL_INFO, "安全沙箱服务启动");
    return TRUE;
}

void CFlashSecurity::Stop( void )
{
    closesocket(m_ListenSock);
    m_ListenSock = INVALID_SOCKET;
    if(NULL != m_hWorkThread)
    {
        WaitForSingleObject(m_hWorkThread, INFINITE);
        CloseHandle(m_hWorkThread);
        m_hWorkThread = NULL;
    }
    LOG(LEVEL_INFO, "安全沙箱服务退出");
}

DWORD WINAPI CFlashSecurity::WorkThread( LPVOID lParam )
{
    CFlashSecurity* pThis = (CFlashSecurity*)lParam;
    pThis->WorkProc();
    return 0;
}

void CFlashSecurity::WorkProc( void )
{
    SOCKET		ClientSocket = INVALID_SOCKET;
    sockaddr_in LocalAddr  = { 0 };
    sockaddr_in	ClientAddr = { 0 };
    int			addr_size = sizeof (sockaddr_in);
    char        cRecvBuff[64] = {0};
    int         iRet = 0;

    closesocket(m_ListenSock);
    m_ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    LocalAddr.sin_family      = AF_INET;
    LocalAddr.sin_port        = htons(843);//监听的端口
    LocalAddr.sin_addr.s_addr = htonl (INADDR_ANY);

    if(bind(m_ListenSock, (LPSOCKADDR)&LocalAddr, addr_size) == SOCKET_ERROR)
    {
        LOG(LEVEL_ERROR, "CFlashSecurity--bind失败");
        closesocket(m_ListenSock);
        return;
    }

    if(listen(m_ListenSock, 10) == SOCKET_ERROR)
    {
        LOG(LEVEL_ERROR, "CFlashSecurity--listen失败");
        closesocket(m_ListenSock);
        return;
    }

    while (INVALID_SOCKET != m_ListenSock)
    {
        ClientSocket = accept(m_ListenSock, (LPSOCKADDR) &ClientAddr, &addr_size);
        if (ClientSocket == INVALID_SOCKET)
        {
            return;
        }
        else
        {
            iRet = recv(ClientSocket, cRecvBuff, 64, 0);
            cRecvBuff[iRet] = '\0';
            if (0 == strcmp(cRecvBuff, "<policy-file-request/>"))  
            {  
                iRet = send(ClientSocket, m_cXML, strlen(m_cXML) + 1, 0);
            }  
            LOG(LEVEL_INFO, "一个安全沙箱请求%s:%d", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));
            //g_pDlg->ShowLog("一个安全沙箱请求%s:%d", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));
            closesocket(ClientSocket);
        }
    }
}
