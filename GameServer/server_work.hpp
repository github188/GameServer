#ifndef BAS_GAME_SERVER_WORK_HPP
#define BAS_GAME_SERVER_WORK_HPP
#include "CommonDefine.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "bas/service_handler.hpp"
#include "bas/io_buffer.hpp"
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

typedef struct tagHeartData
{
    char cData[1024];
    unsigned int iLen;

    tagHeartData()
    {
        memset(cData, 0, 64);
        iLen = 0;
    }
}HeartData;

namespace GameServer {
    class class_room;
    class server_work;
    typedef bas::service_handler<server_work> server_handler_type;
    class server_logic
    {
    public:
        server_logic()
        {
            gm_handle_ = NULL;
        }
        ~server_logic()
        {
            if(NULL != gm_handle_)
            {
                gm_handle_->close(boost::asio::error::connection_refused);
                gm_handle_ = NULL;
            }
        }

    public:
        //处理完成数据包
        void on_packet(server_handler_type& handler);
        //客户端断开
        void on_disconnect(server_handler_type& handler);
        //登陆
        void UserJoinRoom(server_handler_type& handler);
        //学生登录
        void StudentJionRoom(server_handler_type& handler);
        //老师登录
        void TeacherJoinRoom(server_handler_type& handler);
        //数据转发
        void ForwardUserData(server_handler_type& handler);
        //心跳
        void Heartbeat(server_handler_type& handler);
        //请求同步数据
        void RequestSyn(server_handler_type& handler);
        //GM登录
        void GMLogin(server_handler_type& handler);

        //重连协议
        void UserReconnect(server_handler_type& handler);
        //用户确认课程是否完成
        void UserConfirm(server_handler_type& handler);

        map<string, class_room> class_rooms_;
        CLock class_rooms_lock_;
        //GM句柄
        server_handler_type* gm_handle_;
    };

    
    extern GameServer::server_logic* g_pserver_logic;
    
    typedef map<string, class_room>::iterator room_iter;
    class server_work
    {
    public:
        typedef bas::service_handler<server_work> server_handler_type;
        typedef bas::io_buffer Buffers;
        server_work()
        {
        }

        void on_clear(server_handler_type& handler)
        {
        }

        void on_open(server_handler_type& handler)
        {
            handler.last_io_time_ = GetTickCount ();
            LOG(LEVEL_INFO, "新连接:%s", handler.socket().remote_endpoint().address().to_string().c_str());
            //新连接到来，投递一个登陆包的长度
            handler.async_read((size_t)(DATAHEADRELEN + 16));
        }

        void on_read(server_handler_type& handler, std::size_t bytes_transferred)
        {
            handler.last_io_time_ = GetTickCount ();
            handler.read_buffer().produce(bytes_transferred); //刷新buffer长度
            //检查包头是否接收完
            if(handler.read_buffer().size() < DATAHEADRELEN)
            {
                handler.async_read((size_t)DATAHEADRELEN - handler.read_buffer().size()); //继续收包头
                return;
            }
            //检查包体是否收完
            LPDataHeader pHead = (LPDataHeader)handler.read_buffer().data();
            if(handler.read_buffer().size() - DATAHEADRELEN < pHead->Length)
            {
                handler.async_read(pHead->Length - (handler.read_buffer().size() - DATAHEADRELEN)); //接续接收包体
                return;
            }

            //一个完整的包
            if(NULL != g_pserver_logic)
            {
                g_pserver_logic->on_packet(handler);
            }
            
            //处理完成，投递一个接收，接收包头
            handler.read_buffer().clear();
            handler.async_read((size_t)DATAHEADRELEN);
        }

        //一个写完成
        void on_write(server_handler_type& handler, std::size_t bytes_transferred)
        {
            handler.last_io_time_ = GetTickCount ();
            //写完成，清空缓冲
            //handler.write_buffer().clear();
            //handler.write_buffer().consume(bytes_transferred);
            //if(handler.write_buffer().)
            //handler.write_buffer().crunch();
            //TRACE("数据长度：%d", handler.write_buffer().size());
            handler.bip_buffer_.DecommitBlock(bytes_transferred);
        }

        void on_close(server_handler_type& handler, const boost::system::error_code& e)
        {
            switch (e.value())
            {
                // Operation successfully completed.
            case 0:
            case boost::asio::error::eof:
                break;

                // Connection breaked.
            case boost::asio::error::connection_aborted:
            case boost::asio::error::connection_reset:
            case boost::asio::error::connection_refused:
                break;

                //
            case boost::asio::error::already_connected:
                return;
                break;

                // Time out.
            case boost::asio::error::timed_out:
                break;
            
                //接收缓冲溢出，清空一下
            case boost::asio::error::no_buffer_space:
                {
                    LOG(LEVEL_WARNING, "缓冲区满 error:%d message:%s", e.value(), e.message().c_str());
                }
                break;    
                //Other Err
            default:
                LOG(LEVEL_INFO, "error:%d message:%s", e.value(), e.message().c_str());
                break;
            }
            LOG(LEVEL_INFO, "on_close error:%d message:%s", e.value(), e.message().c_str());
            //给更上一层处理
            if(NULL != g_pserver_logic)
            {
                g_pserver_logic->on_disconnect(handler);
            }
            
        }

        void on_parent(server_handler_type& handler, const bas::event event)
        {
        }

        void on_child(server_handler_type& handler, const bas::event event)
        {
        }
        
    };

    class class_room{
        typedef bas::service_handler<server_work> server_handler_type;
    public:
        class_room(string room_key,
            unsigned int start_time,
            unsigned int end_time)
            : room_key_(room_key)
            , start_time_(start_time)
            , end_time_(end_time)
            , teacher_handle_(NULL)
        {
        }
        ~class_room()
        {
            //student_handles_.clear();
        }


        //通过ID查找客户端指针
        server_handler_type* find_client(unsigned int user_id);
        //发送给房间里除自己的所有其他人
        void send_to_all(unsigned int user_id, int iType, char* pBuffer, int iLen);
        //一个用户退出
        void client_disconnent(server_handler_type& handler);

    public:
        //room关键字
        string room_key_;
        //开始时间
        unsigned int start_time_;
        //结束时间
        unsigned int end_time_;
        //学生句柄列表
        list<server_handler_type*> student_handles_;
        //老师句柄
        server_handler_type* teacher_handle_;
        //同步数据
        HeartData heart_data_;
    }; // class class_room

    typedef bas::service_handler<server_work> server_handler_type;
    void send_response(server_handler_type& handler, int iType, char* pBuffer, int iLen);
    void send_heartdata(server_handler_type& handler, class_room& class_room_);
} // namespace GameServer

#endif // BAS_GAME_SERVER_WORK_HPP
