#ifndef server_room_h__
#define server_room_h__
#include <string>
#include <list>
using namespace std;

#include "CommonDefine.h"
#include "bas/service_handler.hpp"
//#include "server_work.hpp"
using bas::service_handler;

namespace GameServer{
//template<typename Work_Handler, typename Socket_Service = boost::asio::ip::tcp::socket>
//class class_room{
//    //typedef service_handler<Work_Handler, Socket_Service> service_handler_type;
//    //typedef boost::shared_ptr<service_handler_type> service_handler_ptr;
//    typedef bas::service_handler server_handler_type;
//public:
//    class_room(string room_key,
//        unsigned int start_time,
//        unsigned int end_time)
//        : room_key_(room_key)
//        , start_time_(start_time)
//        , end_time_(end_time)
//    {
//    }
//    ~class_room()
//    {
//        client_handles_.clear();
//    }
//
//private:
//    //room关键字
//    string room_key_;
//    //开始时间
//    unsigned int start_time_;
//    //结束时间
//    unsigned int end_time_;
//    //客户句柄列表
//    //list<server_handler_type*> client_handles_;
//
//}; // class class_room

//template<typename Work_Handler, typename Socket_Service = boost::asio::ip::tcp::socket>
//class class_room_pool
//{
//    typedef class_room<Work_Handler, Socket_Service> class_room_type;
//    typedef boost::shared_ptr<class_room_type> class_room_ptr;
//
//public:
//    class_room_pool()
//    {
//    }
//    ~class_room_pool()
//    {
//        
//    }
//};

}// namespace GameServer
#endif // server_room_h__
