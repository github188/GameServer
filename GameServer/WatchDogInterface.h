/**********************************************************
* Copyright 2008-2011 HIKVISION Digital Technology Co., Ltd.  
*
* FileName:     WatchDogInterface.h
*
* Description:  watch dog interface
*
* Modification History: 添加了重启软件的命令
*
* <version>    <time>       <author>       <desc>
*   1.0       2008-05-14      lly          create
*   2.0       2009-08-20      lly          modify
***********************************************************/
#ifndef WATCHDOGINTERFACE_H_
#define WATCHDOGINTERFACE_H_ 

// NORMAL-正常, DOG_RESTART - 重启软件, DOG_EXIT-退出
enum DOG_COMMAND{DOG_NORMAL, DOG_RESTART, DOG_EXIT};

// DOG_ACTIVE - 看门狗进程存在, DOG_NOTACTIVE - 看门狗进程不存在
enum DOG_STATUS{DOG_ACTIVE, DOG_NOTACTIVE};

#define HK_WATCHDOG_API extern "C" __declspec(dllimport)

/******************************************************
* Function: OpenWatchDog
*
* Description; 打开看门狗，在程序启动时调用
*
* Parameter: lInterval-心跳时间间隔，范围[1s, 600s],建议5s <= lInterval <=60s
*
* Return: true-成功, false-失败
******************************************************/
HK_WATCHDOG_API bool __stdcall OpenWatchDog(long lInterval);

/******************************************************
* Function: SendHeartbeat
*
* Description; 向看门狗发送心跳，发送心跳的间隔是 lInterval
*
* Parameter: eDogCommand - 发送的命令
*
* Return: true-成功, false-失败
******************************************************/
HK_WATCHDOG_API bool __stdcall SendHeartbeat(DOG_COMMAND eDogCommand);

/******************************************************
* Function: GetWatchdogStatus
*
* Description; 获取看门狗状态
*
* Parameter: void
*
* Return: DOG_ACTIVE - 看门狗进程存在, DOG_NOTACTIVE - 看门狗进程不存在
******************************************************/
HK_WATCHDOG_API DOG_STATUS __stdcall GetWatchdogStatus(void);

#endif