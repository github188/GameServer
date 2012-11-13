/**********************************************************
* Copyright 2008-2011 HIKVISION Digital Technology Co., Ltd.  
*
* FileName:     WatchDogInterface.h
*
* Description:  watch dog interface
*
* Modification History: ������������������
*
* <version>    <time>       <author>       <desc>
*   1.0       2008-05-14      lly          create
*   2.0       2009-08-20      lly          modify
***********************************************************/
#ifndef WATCHDOGINTERFACE_H_
#define WATCHDOGINTERFACE_H_ 

// NORMAL-����, DOG_RESTART - �������, DOG_EXIT-�˳�
enum DOG_COMMAND{DOG_NORMAL, DOG_RESTART, DOG_EXIT};

// DOG_ACTIVE - ���Ź����̴���, DOG_NOTACTIVE - ���Ź����̲�����
enum DOG_STATUS{DOG_ACTIVE, DOG_NOTACTIVE};

#define HK_WATCHDOG_API extern "C" __declspec(dllimport)

/******************************************************
* Function: OpenWatchDog
*
* Description; �򿪿��Ź����ڳ�������ʱ����
*
* Parameter: lInterval-����ʱ��������Χ[1s, 600s],����5s <= lInterval <=60s
*
* Return: true-�ɹ�, false-ʧ��
******************************************************/
HK_WATCHDOG_API bool __stdcall OpenWatchDog(long lInterval);

/******************************************************
* Function: SendHeartbeat
*
* Description; ���Ź��������������������ļ���� lInterval
*
* Parameter: eDogCommand - ���͵�����
*
* Return: true-�ɹ�, false-ʧ��
******************************************************/
HK_WATCHDOG_API bool __stdcall SendHeartbeat(DOG_COMMAND eDogCommand);

/******************************************************
* Function: GetWatchdogStatus
*
* Description; ��ȡ���Ź�״̬
*
* Parameter: void
*
* Return: DOG_ACTIVE - ���Ź����̴���, DOG_NOTACTIVE - ���Ź����̲�����
******************************************************/
HK_WATCHDOG_API DOG_STATUS __stdcall GetWatchdogStatus(void);

#endif