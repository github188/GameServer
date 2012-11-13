// Client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#pragma  comment(lib,"ws2_32.lib")
#include <iostream>
using namespace std;

typedef struct tag_Data_Header
{
    unsigned short int Cmd;       
    unsigned short int Length;   //数据包长度(不包括包头),2字节
}DataHeader, *LPDataHeader;

DWORD _stdcall ConnectThread(LPVOID lParam);
DWORD _stdcall WorkThread(LPVOID lParam);

unsigned int start_time = 1000;
unsigned int end_time = 2000;
unsigned int teacher_id = 101;
unsigned int student_id = 1001;
char ip[32];
bool bOk = false;



int _tmain(int argc, _TCHAR* argv[])
{
    WSADATA lpWSAData;
    WSAStartup(MAKEWORD(2, 2), &lpWSAData);	
    HANDLE hThread = NULL;
    int iNum = 10;

    cout<<"服务器IP:";
    cin>>ip;
    cout<<"房间数:";
    cin>>iNum;
    cout<<"老师开始ID:";
    cin>>teacher_id;

    printf("房间数%d, 开始连接...\n", iNum);
    for (int i = teacher_id; i < teacher_id + iNum; i++)
    {
        CreateThread(NULL, NULL, ConnectThread, (LPVOID)i, NULL, NULL);
        Sleep(100);
    }
    printf("房间创建完成，开始转发数据...\n");
    bOk = true;
    //ConnectThread (NULL);

    Sleep(10000000);
    
    
    
    WSACleanup();

	return 0;
}

SOCKET TeacherConn(int Id)
{
    int iRet = 0;
    sockaddr_in LocalAddr;
    LocalAddr.sin_family=AF_INET;
    LocalAddr.sin_port=htons(11543);
    LocalAddr.sin_addr.S_un.S_addr=inet_addr(ip);
    SOCKET sClient = INVALID_SOCKET;

    sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(SOCKET_ERROR == connect(sClient,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)))
    {
        closesocket(sClient);
        return INVALID_SOCKET;
    }

    char cBuff[1024] = "";
    LPDataHeader pHeader = (LPDataHeader)cBuff;
    pHeader->Cmd = 1;
    pHeader->Length = 16;

    memcpy(cBuff + 4, &Id, 4);
    //memcpy(cBuff + 8, &student_id, 4);
    memcpy(cBuff + 12, &start_time, 4);
    memcpy(cBuff + 16, &end_time, 4);

    iRet = send(sClient, cBuff, pHeader->Length + 4, 0);
    if(iRet <= 0)
    {
        printf("-------------%d\n", WSAGetLastError());
    }
    recv(sClient, cBuff, 1024, 0);
    return sClient;
}

SOCKET StudentConn(int Id)
{
    int iRet = 0;
    sockaddr_in LocalAddr;
    LocalAddr.sin_family=AF_INET;
    LocalAddr.sin_port=htons(11543);
    LocalAddr.sin_addr.S_un.S_addr=inet_addr(ip);
    SOCKET sClient = INVALID_SOCKET;

    sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(SOCKET_ERROR == connect(sClient,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)))
    {
        closesocket(sClient);
        printf("连接失败");
        return INVALID_SOCKET;
    }

    char cBuff[1024] = "";
    LPDataHeader pHeader = (LPDataHeader)cBuff;
    pHeader->Cmd = 1;
    pHeader->Length = 16;

    int s_id = Id * 10000;

    memcpy(cBuff + 4, &Id, 4);
    memcpy(cBuff + 8, &s_id, 4);
    memcpy(cBuff + 12, &start_time, 4);
    memcpy(cBuff + 16, &end_time, 4);

    iRet = send(sClient, cBuff, pHeader->Length + 4, 0);
    if(iRet <= 0)
    {
        printf("-------------%d\n", WSAGetLastError());
    }
    recv(sClient, cBuff, 1024, 0);
    return sClient;
}

int ForwardUserData(SOCKET sTeacher, SOCKET sStudent)
{
    int iRet = 0;
    char cBuff[1024] = {0};
    LPDataHeader pHeader = (LPDataHeader)cBuff;
    pHeader->Cmd = 3;
    pHeader->Length = 32;
    
    strcpy(cBuff + 6, "123456789012345678901");
    
    iRet = send(sTeacher, cBuff, pHeader->Length + 4, 0);
    if(iRet <= 0)
    {
        printf("----%d\n", WSAGetLastError());
    }
    memset(cBuff, 0, 1024);
    recv(sStudent, cBuff, 1024, 0);
    if(bOk)
    {
        printf(cBuff + 4);
        printf(" ");
    }
    return 0;
}

DWORD _stdcall ConnectThread(LPVOID lParam)
{
    int iTId = (int )lParam;
    printf("创建第%d房间, 老师ID%d\n", iTId - teacher_id, iTId);
    SOCKET sTeacher = TeacherConn(iTId);
    SOCKET sStudent = StudentConn(iTId);

    for (;;)
    {
        ForwardUserData(sTeacher, sStudent);
        Sleep(300);
    }
    
    //while(1)
    //{
    //    ZeroMemory(cBuff, 1024);
    //    strcpy(cBuff, "1234567890");
    //    iRet = send(sClient, cBuff, strlen(cBuff), 0);
    //    if(iRet <= 0)
    //    {
    //        break;
    //    }
    //    ZeroMemory(cBuff, 1024);
    //    //recv(sClient, cBuff, 1024, 0);
    //    //iRet = send(sClient, cBuff, strlen(cBuff), 0);
    //    printf(cBuff);
    //    printf("\n");
    //    Sleep(5000);
    //   
    //}
    //closesocket(sClient);
    printf("断开连接\n");
    return 0;
}