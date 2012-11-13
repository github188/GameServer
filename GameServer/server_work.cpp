#include "stdafx.h"
#include "server_work.hpp"

GameServer::server_logic* GameServer::g_pserver_logic = NULL;


string HexDump(const char *buf, int nBufLen)    
{       
    string strRtn = "";
    char buff[32] = {0};
    for(int i = 0; i < nBufLen; i++)      
    {         
        sprintf_s(buff, 32, "0x%gt", buf[i]); 
        strRtn.append(buff);
    }      
    return strRtn;
}     

void GameServer::server_logic::on_packet(server_handler_type& handler)
{
    LPDataHeader pHeader = (LPDataHeader)handler.read_buffer().data();

    switch (pHeader->Cmd)
    {
    case CMD_USER_JOIN:
        {
            UserJoinRoom(handler);
        }
        break;    
    case CMD_FORWARDDATA_REQUEST:
        {
            ForwardUserData(handler);
        }
        break;
    case CMD_HEARTBEAT:
        {
            Heartbeat(handler);
        }
        break;
    case CMD_USER_RECONNECT:
        {
            UserReconnect(handler);
        }
        break;
    /*case CMD_GM_LOGIN:
        {
            GMLogin(handler);
        }
        break;*/
    case CMD_USER_CONFIRM:
        {
            UserConfirm(handler);
        }
        break;
    default:
        {
            handler.close(boost::asio::error::connection_refused);
            LOG(LEVEL_WARNING, "未知数据类型, userid:%d", handler.user_id_);
        }
        break;
    }
}

void GameServer::server_logic::on_disconnect(server_handler_type& handler)
{
    room_iter iter;
    LOG(LEVEL_INFO, "on_disconnect ID:%d", handler.user_id_);

    try
    {

        iter = class_rooms_.find(handler.room_key_);

        if(iter == class_rooms_.end())
        {
            return;
        }

        //处理下线
        iter->second.client_disconnent(handler);

        ////看房间里还有没有人
        //if(NULL == iter->second.teacher_handle_
        //    && 0 == iter->second.student_handles_.size())
        //{
        //    LOG(LEVEL_INFO, "删除房间,Key:%s", iter->first.c_str());
        //    room_lock_.Lock();
        //    class_rooms_.erase(iter);
        //    room_lock_.Unlock();
        //}
    }
    catch(...)
    {
        LOG(LEVEL_WARNING, "on_disconnect函数捕获一次异常,Code:%d", GetLastError());
    }
}

void GameServer::server_logic::UserJoinRoom(server_handler_type& handler)
{
    unsigned char* pData = (unsigned char*)(handler.read_buffer().data() + DATAHEADRELEN);
    int iID = 0;
    memcpy(&iID, pData + 4, 4);

    if(0 == iID)
    {
        TeacherJoinRoom(handler);
    }
    else
    {
        StudentJionRoom(handler);
    }
}

void GameServer::server_logic::StudentJionRoom(server_handler_type& handler)
{
    room_iter iter;
    ostringstream stm;
    string str_room_key = "";
    unsigned int start_time = 0;
    unsigned int end_time = 0;
    unsigned int teacher_id = 0;
    unsigned int student_id = 0;
    int iClassId = 0;
    char cBuff[64] = {0};
    LPDataHeader pHeader = (LPDataHeader)handler.read_buffer().data();
    unsigned char* pData = (unsigned char*)(handler.read_buffer().data() + DATAHEADRELEN);

    memcpy(&teacher_id, pData, 4);
    memcpy(&student_id, pData + 4, 4);
    memcpy(&start_time, pData + 8, 4);
    memcpy(&end_time, pData + 12, 4);
    memcpy(&iClassId, pData + 16, 4);

    LOG(LEVEL_INFO, "StudentJionRoom teacher_id:%d student_id:%d start_time:%d end_time:%d", teacher_id, student_id, start_time, end_time);

    try
    {
        stm.str("");
        stm<<teacher_id<<start_time;
        str_room_key = stm.str();
        iter = class_rooms_.find(str_room_key);

        handler.room_key_ = str_room_key;
        handler.user_type_ = 2;
        handler.user_id_ = student_id;
        handler.class_id_ = iClassId;

        //房间还未建立
        if(iter == class_rooms_.end())
        {
            LOG(LEVEL_INFO, "创建房间:%s", str_room_key.c_str());
            class_room new_room(str_room_key, start_time, end_time);
            new_room.student_handles_.push_back(&handler);
            class_rooms_lock_.Lock();
            class_rooms_.insert(pair<string, class_room>(str_room_key, new_room));
            class_rooms_lock_.Unlock();

            
            //10字节回执
            memset(cBuff, 0, 64);
            short int iLen = 1;
            //memcpy(cBuff, &teacher_id, 4);
            memcpy(cBuff + 4, &iLen, 2);
            memcpy(cBuff + 6, &student_id, 4);
            GameServer::send_response(handler, CMD_JOIN_SUCCES, cBuff, 10);
            LOG(LEVEL_INFO, "学生登录id:%d\n", student_id);
            
            
            theApp.m_pADOOperation->RecordClassLog(student_id, 2, iClassId, 1);
        }
        else
        {
            server_handler_type* phandler = iter->second.find_client(student_id);
            iter->second.student_handles_.push_back(&handler);
            if(NULL != phandler) //重复登录
            {
                iter->second.student_handles_.remove(phandler);
                LOG(LEVEL_WARNING, "学生重复登录, 剔除: id:%d 课程id:%d", phandler->user_id_, phandler->class_id_);
                phandler->close(boost::asio::error::already_connected);
            }
            //1字节用户类型（0老师，1学生）4字节上线者ID
            cBuff[0] = 1;
            memcpy(cBuff + 1, &student_id, 4);
            iter->second.send_to_all(student_id, CMD_JOIN_NOTIFY, cBuff, 5); //上线，通知其他人

           
            //回执，上线成功，4字节教师,2字节数组长度，学生id数组
            memset(cBuff, 0, 64);
            if(NULL != iter->second.teacher_handle_)
            {
                memcpy(cBuff, &(iter->second.teacher_handle_->user_id_), 4);
            }
            short int iLen = iter->second.student_handles_ .size();
            memcpy(cBuff + 4, &iLen, 2);
            list<server_handler_type*>::iterator student_iter = iter->second.student_handles_.begin();
            for (short int i = 0; i < iLen; i++)
            {
                memcpy(cBuff + 4 + 2 + 4 * i, &(*student_iter)->user_id_, 4);
                student_iter++;
            }
            send_response(handler, CMD_JOIN_SUCCES, cBuff, 4 + 2 + 4 * iLen);
            LOG(LEVEL_INFO, "学生登录id:%d\n", student_id);
            
            //同步数据
            if(iter->second.heart_data_.iLen > 0)
            {
                send_response(handler, CMD_RECONNECT_SUCC, iter->second.heart_data_.cData, iter->second.heart_data_.iLen);
            }
            theApp.m_pADOOperation->RecordClassLog(student_id, 2, iClassId, 1);
        }
    }
    catch(...)
    {
        handler.close();
        LOG(LEVEL_WARNING, "StudentJionRoom函数捕获一次异常,Code:%d", GetLastError());
    }
}

void GameServer::server_logic::TeacherJoinRoom(server_handler_type& handler)
{
    room_iter iter;
    ostringstream stm;
    string str_room_key = "";
    unsigned int start_time = 0;
    unsigned int end_time = 0;
    unsigned int teacher_id = 0;
    int iClassId = 0;
    char cBuff[64] = {0};
    LPDataHeader pHeader = (LPDataHeader)handler.read_buffer().data();
    unsigned char* pData = (unsigned char*)(handler.read_buffer().data() + DATAHEADRELEN);

    memcpy(&teacher_id, pData, 4);
    memcpy(&start_time, pData + 8, 4);
    memcpy(&end_time, pData + 12, 4);
    memcpy(&iClassId, pData + 16, 4);
    
    LOG(LEVEL_INFO, "TeacherJoinRoom teacher_id:%d start_time:%d end_time:%d", teacher_id, start_time, end_time);
    try
    {
        stm.str("");
        stm<<teacher_id<<start_time;
        str_room_key = stm.str();
        iter = class_rooms_.find(str_room_key);

        handler.room_key_ = str_room_key;
        handler.user_type_ = 1;
        handler.user_id_ = teacher_id;
        handler.class_id_ = iClassId;


        //房间还未建立
        if(iter == class_rooms_.end())
        {
            LOG(LEVEL_INFO, "创建房间:%s", str_room_key.c_str());
            class_room new_room(str_room_key, start_time, end_time);
            new_room.teacher_handle_ = &handler;
            class_rooms_lock_.Lock();
            class_rooms_.insert(pair<string, class_room>(str_room_key, new_room));
            class_rooms_lock_.Unlock();

            
            //10字节回执
            memset(cBuff, 0, 64);
            memcpy(cBuff, &teacher_id, 4);
            GameServer::send_response(handler, CMD_JOIN_SUCCES, cBuff, 6);
            LOG(LEVEL_INFO, "老师登录id:%d\n", teacher_id);
            
            
            theApp.m_pADOOperation->RecordClassLog(teacher_id, 1, iClassId, 1);
        }
        else
        {
            server_handler_type* phandler = iter->second.teacher_handle_;
            iter->second.teacher_handle_ = &handler;

            if(NULL != phandler) //重复登录
            {
                LOG(LEVEL_WARNING, "老师重复登录, 剔除: id:%d 课程id:%d", phandler->user_id_, phandler->class_id_);
                phandler->close(boost::asio::error::already_connected);
                //Sleep(100);
            }

            //1字节用户类型（0老师，1学生）4字节掉线者ID
            cBuff[0] = 0;
            memcpy(cBuff + 1, &teacher_id, 4);
            iter->second.send_to_all(teacher_id, CMD_JOIN_NOTIFY, cBuff, 5); //上线，通知其他人

            //回执，上线成功，4字节教师,2字节数组长度，学生id数组
            memset(cBuff, 0, 64);
            memcpy(cBuff, &teacher_id, 4);
            short int iLen = iter->second.student_handles_.size();
            memcpy(cBuff + 4, &iLen, 2);
            list<server_handler_type*>::iterator student_iter = iter->second.student_handles_.begin();
            for (short int i = 0; i < iLen; i++)
            {
                memcpy(cBuff + 4 + 2 + 4 * i, &((*student_iter)->user_id_), 4);
                student_iter++;
            }
            send_response(handler, CMD_JOIN_SUCCES, cBuff, 4 + 2 + 4 * iLen);
            LOG(LEVEL_INFO, "老师登录id:%d\n", teacher_id);
            
            //发送同步数据
            if(iter->second.heart_data_.iLen > 0)
            {
                send_response(handler, CMD_RECONNECT_SUCC, iter->second.heart_data_.cData, iter->second.heart_data_.iLen);
            }
            
            theApp.m_pADOOperation->RecordClassLog(teacher_id, 1, iClassId, 1);
        }
    }
    catch(...)
    {
        handler.close();
        LOG(LEVEL_WARNING, "TeacherJoinRoom函数捕获一次异常,Code:%d", GetLastError());
    }
}

void GameServer::server_logic::ForwardUserData(server_handler_type& handler)
{
    room_iter iter;
    short int iArrayLen = 0;
    unsigned int user_id = 0;
    server_handler_type* pserver_handler = NULL;
    LPDataHeader pHeader = (LPDataHeader)handler.read_buffer().data();
    char* pData = (char*)(handler.read_buffer().data() + DATAHEADRELEN);

    try
    {
        iter = class_rooms_.find(handler.room_key_);
        if(iter == class_rooms_.end())
        {
            handler.close ();
            return;
        }

        memcpy(&iArrayLen, pData, 2);
        if(0 == iArrayLen)
        {
            LOG(LEVEL_INFO, "ID:%d转发数据给所有人", handler.user_id_);
            iter->second.send_to_all(handler.user_id_, CMD_FORWARDDATA_RESPONSE, pData + 2, pHeader->Length - 2);
            return;
        }

        for (short int i = 0; i < iArrayLen; i++)
        {
            memcpy(&user_id, pData + 2 + i * 4, 4);
            pserver_handler = iter->second.find_client(user_id);
            if(NULL == pserver_handler) 
            {
                continue;
            }
            LOG(LEVEL_INFO, "ID:%d转发数据给ID:%d", handler.user_id_, user_id);
            send_response(*pserver_handler, CMD_FORWARDDATA_RESPONSE, pData + 2 + 4 * iArrayLen, pHeader->Length - 2 - 4 * iArrayLen);
        }
    }
    catch(...)
    {
        LOG(LEVEL_WARNING, "ForwardUserData函数捕获一次异常,Code:%d", GetLastError());
    }
}

void GameServer::server_logic::Heartbeat(server_handler_type& handler)
{
    room_iter iter;
    unsigned int iTime = 0;
    LPDataHeader pHeader = (LPDataHeader)handler.read_buffer().data();
    char* pData = (char*)(handler.read_buffer().data() + DATAHEADRELEN);
    HeartData struData;
    size_t i = 0;
   
    try
    {
        do 
        {
            iter = class_rooms_.find(handler.room_key_);
            if(iter == class_rooms_.end())
            {
                handler.close ();
                return;
            }
                
            //学生心跳直接返回
            if(2 == handler.user_type_ || pHeader->Length > 1023 || pHeader->Length < 7)
            {
                break;
            }

            memcpy(iter->second.heart_data_.cData, pData, pHeader->Length);
            iter->second.heart_data_.iLen = pHeader->Length;

        } while (0);

        iTime = (unsigned int)time(NULL);
        send_response(handler, CMD_HEARTBEAT_RETURN, (char*)&iTime, 4);
    }
    catch(...)
    {   
        LOG(LEVEL_WARNING, "Heartbeat函数捕获一次异常,Code:%d", GetLastError());
    }
}

void GameServer::server_logic::RequestSyn(server_handler_type& handler)
{
    //LPDataHeader pHeader = (LPDataHeader)handler.read_buffer().data();
    //char* pData = (char*)(handler.read_buffer().data() + DATAHEADRELEN);

    /*try
    {
        if(handler.heart_data_len_ > 64)
        {
            handler.heart_data_len_ = 64;
        }
        send_response(handler, CMD_RESPONESYN, (char*)handler.heart_data_, handler.heart_data_len_);
    }
    catch(...)
    {   
        LOG(LEVEL_WARNING, "RequestSyn函数捕获一次异常,Code:%d", GetLastError());
    }*/
}

void GameServer::server_logic::GMLogin(server_handler_type& handler)
{
    LPDataHeader pHeader = (LPDataHeader)handler.read_buffer().data();
    unsigned char* pData = (unsigned char*)(handler.read_buffer().data() + DATAHEADRELEN);
    
    if(NULL != gm_handle_)
    {
        gm_handle_->close(boost::asio::error::connection_refused);
        gm_handle_ = NULL;
    }

    gm_handle_ = &handler;

    //send_response(handler, )
}

void GameServer::server_logic::UserReconnect( server_handler_type& handler )
{
   /* UserJoinRoom(handler);
    char cBuff[64] = {0};
    
    if(handler.heart_data_index_ < 0)
    {
        send_response(handler, CMD_RECONNECT_SUCC, cBuff, 1);
    }

    room_iter iter;
    try
    {
        iter = class_rooms_.find(handler.room_key_);
        if(iter == class_rooms_.end())
        {
            return;
        }
        send_response(handler, CMD_RECONNECT_SUCC, 
            iter->second.heart_data_[handler.heart_data_index_].cData, iter->second.heart_data_[handler.heart_data_index_].iLen);
    }
    catch(...)
    {
        LOG(LEVEL_WARNING, "UserReconnect函数捕获一次异常,发送回执心跳数据错误，Code:%d", GetLastError());
    }*/
}



void GameServer::server_logic::UserConfirm( server_handler_type& handler )
{
    int iClassId = 0;

    LPDataHeader pHeader = (LPDataHeader)handler.read_buffer().data();
    unsigned char* pData = (unsigned char*)(handler.read_buffer().data() + DATAHEADRELEN);

    memcpy(&iClassId, pData, 4);


}


typedef bas::service_handler<GameServer::server_work> server_handler_type;
server_handler_type* GameServer::class_room::find_client(unsigned int user_id)
{
    if(NULL != teacher_handle_ 
        && user_id == teacher_handle_->user_id_)
    {
        return teacher_handle_;
    }
    list<server_handler_type*>::iterator iter = student_handles_.begin();
    for (; iter != student_handles_.end(); iter++)
    {
        if(user_id == (*iter)->user_id_)
        {
            return *iter;
        }
    }
    return NULL;
}

void GameServer::class_room::send_to_all(unsigned int user_id, int iType, char* pBuffer, int iLen)
{
    if(NULL != teacher_handle_ && teacher_handle_->user_id_ != user_id)
    {
        send_response(*teacher_handle_, iType, pBuffer, iLen);
    }
    list<server_handler_type*>::iterator iter = student_handles_.begin();
    for (; iter != student_handles_.end(); iter++)
    {
        if(user_id != (*iter)->user_id_)
        {
            send_response(**iter, iType, pBuffer, iLen);
        }
    }
}

void GameServer::class_room::client_disconnent(server_handler_type& handler)
{ 
    //先剔除用户
    if(NULL != teacher_handle_
        && 1 == handler.user_type_
        && handler.user_id_ == teacher_handle_->user_id_)
    {
        LOG(LEVEL_INFO, "老师退出ID:%d", handler.user_id_);
        teacher_handle_ = NULL;
        theApp.m_pADOOperation->RecordClassLog(handler.user_id_, 1, handler.class_id_, 2);
    }
    list<server_handler_type*>::iterator iter = student_handles_.begin();
    for (; iter != student_handles_.end();)
    {
        if(&handler == *iter)
        {
            LOG(LEVEL_INFO, "学生退出ID:%d", handler.user_id_);
            student_handles_.erase(iter++);
            theApp.m_pADOOperation->RecordClassLog(handler.user_id_, 2, handler.class_id_, 2);
        }
        else
        {
            iter++;
        }
    }
    
    //给其他用户发下线通知,5字节
    char cBuff[16] = {0};
    if(1 == handler.user_type_)
    {
        cBuff[0] = 0;
    }
    else
    {
        cBuff[0] = 1;
    }
    memcpy(cBuff + 1, &handler.user_id_, 4);
    send_to_all(handler.user_id_, CMD_REMOVE_NOTIFY, cBuff, 5);
}

void GameServer::send_response(server_handler_type& handler, int iType, char* pBuffer, int iLen)
{
    /*handler.write_buffer().clear();
    unsigned char* pData = handler.write_buffer().data();
    int iRet = 0;
    LPDataHeader pHeader = (LPDataHeader)pData;
    pHeader->Cmd = iType;
    pHeader->Length = iLen;
    memcpy(pData + DATAHEADRELEN, pBuffer, iLen);
    handler.write_buffer().produce(DATAHEADRELEN + iLen);
    LOG(LEVEL_INFO, "发送数据,协议%d 长度%d To:%d Body:%s", iType, iLen, handler.user_id_, HexDump((const char*)handler.write_buffer().data() + DATAHEADRELEN, iLen).c_str());
    handler.async_write();*/

    int iSpace = 0;
    unsigned char* pData = handler.bip_buffer_.Reserve(handler.bip_buffer_.GetBufferSize(), iSpace);
    if(iSpace < DATAHEADRELEN + iLen || NULL == pData)
    {
        LOG(LEVEL_WARNING, "发送缓冲区不足");
        return;
    }
    handler.bip_buffer_.Commit(DATAHEADRELEN + iLen);
    LPDataHeader pHeader = (LPDataHeader)pData;
    pHeader->Cmd = iType;
    pHeader->Length = iLen;
    memcpy(pData + DATAHEADRELEN, pBuffer, iLen);
    handler.async_write(boost::asio::buffer(pData, DATAHEADRELEN + iLen));
}

void GameServer::send_heartdata(server_handler_type& handler, class_room& class_room_)
{
    /*size_t i = 0;
    char buffer[64] = {0};

    for (i = 0; i < class_room_.heart_data_.size (); i++)
    {
        if(class_room_.heart_data_[i].iID == handler.user_id_)
        {
            send_response(handler, CMD_RECONNECT_SUCC, 
                class_room_.heart_data_[i].cData, class_room_.heart_data_[i].iLen);
            break;
        }
    }

    if(i == class_room_.heart_data_.size ())
    {
        send_response (handler, CMD_RECONNECT_SUCC, )
    }*/
}
