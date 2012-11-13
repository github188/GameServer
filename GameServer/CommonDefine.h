#ifndef CommonDefine_h__
#define CommonDefine_h__

#include <assert.h>
#include <map>
#include <list>
#include <vector>

using namespace std;

#define DATAHEADRELEN       4           //数据头长度

//////网络通信命令定义//////
#define CMD_USER_JOIN                       1
//#define CMD_STUDENT_JOIN                    2
#define CMD_FORWARDDATA_REQUEST             3
#define CMD_HEARTBEAT                       4
#define CMD_REQUESTSYN                      5

#define CMD_GM_LOGIN                        7
#define CMD_GM_JOIN                         8

#define CMD_USER_RECONNECT                  10
#define CMD_USER_CONFIRM                    11


#define CMD_JOIN_SUCCES                     101
#define CMD_FORWARDDATA_RESPONSE            103
#define CMD_HEARTBEAT_RETURN                104
#define CMD_RESPONESYN                      105

#define CMD_JOIN_NOTIFY                     106
#define CMD_REMOVE_NOTIFY                   107

#define CMD_RECONNECT_SUCC                  110


#define CMD_ERROR_RETURN                    500

//////////////////////////////////////////



typedef struct tag_Data_Header
{
    unsigned short int Cmd;       
    unsigned short int Length;   //数据包长度(不包括包头),2字节
}DataHeader, *LPDataHeader;



//typedef struct tag_Class_Room
//{
//    DWORD               dwStartTime;             //课程开始时间
//    DWORD               dwEndTime;               //课程结束时间
//    LPClientInfo        lpTeacher;              //教师指针
//    list<LPClientInfo>  StudentList;            //学生列表
//    
//    tag_Class_Room()
//    {
//        dwStartTime = 0;
//        dwEndTime = 0;
//        lpTeacher = NULL;
//    }
//}ClassRoom, *LPClassRoom;


#endif // CommonDefine_h__
