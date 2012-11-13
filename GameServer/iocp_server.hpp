#ifndef iocp_server_h__
#define iocp_server_h__

#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/basic_deadline_timer.hpp>

#include "bas/server.hpp"

#include "server_work.hpp"
#include "server_work_allocator.hpp"

using namespace std;

typedef bas::server<GameServer::server_work, GameServer::server_work_allocator> server;
typedef bas::service_handler_pool<GameServer::server_work, GameServer::server_work_allocator> service_handler_pool_type;


namespace GameServer{

class iocp_server
{
public:
    iocp_server()
        : m_pioser(NULL)
        , m_bStart(false)
    {
    }
    ~iocp_server()
    {
        if(NULL != m_pioser)
        {
            delete m_pioser;
            m_pioser = NULL;
        }
    }

    bool IsStart(void)
    {
        return m_bStart;
    }
    
    bool Start(unsigned short port,
        size_t io_pool_size, 
        size_t work_pool_init_size,
        size_t work_pool_high_watermark,
        size_t preallocated_handler_number,
        size_t read_buffer_size,
        size_t write_buffer_size,
        size_t session_timeout,
        size_t io_timeout)
    {
        if(NULL == g_pserver_logic)
        {
            g_pserver_logic = new server_logic;
        }

        if(NULL == m_pioser)
        {
            m_pioser = new server("0.0.0.0",
                port,
                io_pool_size,
                work_pool_init_size,
                work_pool_high_watermark,
                new service_handler_pool_type(new GameServer::server_work_allocator(),
                preallocated_handler_number,
                read_buffer_size,
                write_buffer_size,
                session_timeout,
                0));
        }
        if(NULL == m_pioser)
        {
            return false;
        }
        io_timeout_ = io_timeout;
        m_bStart = true;
        m_pioser->run();
        m_bStart = false;
        return true;
    }

    void Stop()
    {
        if(NULL != m_pioser)
        {
            m_pioser->stop();
        }
        if(NULL != g_pserver_logic)
        {
            delete g_pserver_logic;
            g_pserver_logic = NULL;
        }
        m_bStart = false;
    }
   
    void check_timeout()
    {
        if(!m_bStart)
        {
            return;
        }
        int iTime = GetTickCount();
        server_handler_type* handler_= NULL;
        LOG(LEVEL_OUTPUT, "-----------------进入check_timeout");

        //拷贝一份，提高速度
        g_pserver_logic->class_rooms_lock_.Lock ();
        map<string, class_room> class_rooms_tmp_ = g_pserver_logic->class_rooms_;
        g_pserver_logic->class_rooms_lock_.Unlock ();

        room_iter iter = class_rooms_tmp_.begin ();
        for (; iter != class_rooms_tmp_.end (); iter++)
        {
            if(NULL != iter->second.teacher_handle_ 
                && GetTickCount() - iter->second.teacher_handle_->last_io_time_ > 1000 * io_timeout_)
            {
                handler_ = iter->second.teacher_handle_;
                iter->second.teacher_handle_ = NULL;
                LOG(LEVEL_INFO, "超时，删除老师%d", handler_->user_id_);
                handler_->close (boost::asio::error::timed_out);
            }

            list<server_handler_type*>::iterator iter_stu = iter->second.student_handles_.begin ();
            for (; iter_stu != iter->second.student_handles_.end ();)
            {
                if(NULL != (*iter_stu) && GetTickCount () - (*iter_stu)->last_io_time_ > 1000 * io_timeout_)
                {
                    handler_ = (*iter_stu);
                    iter->second.student_handles_.erase(iter_stu++);
                    LOG(LEVEL_INFO, "超时，删除学生%d", handler_->user_id_);
                    handler_->close (boost::asio::error::timed_out);
                }
                else
                {
                    iter_stu++;
                }
            }
        }
        LOG(LEVEL_OUTPUT, "---------------退出check_timeout，%dms", GetTickCount() - iTime);
    }



private:
    server* m_pioser;
    bool m_bStart;
    unsigned int io_timeout_;
};

}// namespace GameServer
#endif // iocp_server_h__
