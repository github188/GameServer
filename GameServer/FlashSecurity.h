#pragma once

class CFlashSecurity
{
public:
    CFlashSecurity(void);
    ~CFlashSecurity(void);

public:
    BOOL Start(void);
    void Stop(void);

private:
    static DWORD WINAPI WorkThread(LPVOID lParam);
    void WorkProc(void);

    SOCKET m_ListenSock;
    HANDLE m_hWorkThread;
    char   m_cXML[256];
};
