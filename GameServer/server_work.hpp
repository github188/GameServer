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
        //����������ݰ�
        void on_packet(server_handler_type& handler);
        //�ͻ��˶Ͽ�
        void on_disconnect(server_handler_type& handler);
        //��½
        void UserJoinRoom(server_handler_type& handler);
        //ѧ����¼
        void StudentJionRoom(server_handler_type& handler);
        //��ʦ��¼
        void TeacherJoinRoom(server_handler_type& handler);
        //����ת��
        void ForwardUserData(server_handler_type& handler);
        //����
        void Heartbeat(server_handler_type& handler);
        //����ͬ������
        void RequestSyn(server_handler_type& handler);
        //GM��¼
        void GMLogin(server_handler_type& handler);

        //����Э��
        void UserReconnect(server_handler_type& handler);
        //�û�ȷ�Ͽγ��Ƿ����
        void UserConfirm(server_handler_type& handler);

        map<string, class_room> class_rooms_;
        CLock class_rooms_lock_;
        //GM���
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
            LOG(LEVEL_INFO, "������:%s", handler.socket().remote_endpoint().address().to_string().c_str());
            //�����ӵ�����Ͷ��һ����½���ĳ���
            handler.async_read((size_t)(DATAHEADRELEN + 16));
        }

        void on_read(server_handler_type& handler, std::size_t bytes_transferred)
        {
            handler.last_io_time_ = GetTickCount ();
            handler.read_buffer().produce(bytes_transferred); //ˢ��buffer����
            //����ͷ�Ƿ������
            if(handler.read_buffer().size() < DATAHEADRELEN)
            {
                handler.async_read((size_t)DATAHEADRELEN - handler.read_buffer().size()); //�����հ�ͷ
                return;
            }
            //�������Ƿ�����
            LPDataHeader pHead = (LPDataHeader)handler.read_buffer().data();
            if(handler.read_buffer().size() - DATAHEADRELEN < pHead->Length)
            {
                handler.async_read(pHead->Length - (handler.read_buffer().size() - DATAHEADRELEN)); //�������հ���
                return;
            }

            //һ�������İ�
            if(NULL != g_pserver_logic)
            {
                g_pserver_logic->on_packet(handler);
            }
            
            //������ɣ�Ͷ��һ�����գ����հ�ͷ
            handler.read_buffer().clear();
            handler.async_read((size_t)DATAHEADRELEN);
        }

        //һ��д���
        void on_write(server_handler_type& handler, std::size_t bytes_transferred)
        {
            handler.last_io_time_ = GetTickCount ();
            //д��ɣ���ջ���
            //handler.write_buffer().clear();
            //handler.write_buffer().consume(bytes_transferred);
            //if(handler.write_buffer().)
            //handler.write_buffer().crunch();
            //TRACE("���ݳ��ȣ�%d", handler.write_buffer().size());
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
            
                //���ջ�����������һ��
            case boost::asio::error::no_buffer_space:
                {
                    LOG(LEVEL_WARNING, "�������� error:%d message:%s", e.value(), e.message().c_str());
                }
                break;    
                //Other Err
            default:
                LOG(LEVEL_INFO, "error:%d message:%s", e.value(), e.message().c_str());
                break;
            }
            LOG(LEVEL_INFO, "on_close error:%d message:%s", e.value(), e.message().c_str());
            //������һ�㴦��
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


        //ͨ��ID���ҿͻ���ָ��
        server_handler_type* find_client(unsigned int user_id);
        //���͸���������Լ�������������
        void send_to_all(unsigned int user_id, int iType, char* pBuffer, int iLen);
        //һ���û��˳�
        void client_disconnent(server_handler_type& handler);

    public:
        //room�ؼ���
        string room_key_;
        //��ʼʱ��
        unsigned int start_time_;
        //����ʱ��
        unsigned int end_time_;
        //ѧ������б�
        list<server_handler_type*> student_handles_;
        //��ʦ���
        server_handler_type* teacher_handle_;
        //ͬ������
        HeartData heart_data_;
    }; // class class_room

    typedef bas::service_handler<server_work> server_handler_type;
    void send_response(server_handler_type& handler, int iType, char* pBuffer, int iLen);
    void send_heartdata(server_handler_type& handler, class_room& class_room_);
} // namespace GameServer

#endif // BAS_GAME_SERVER_WORK_HPP
