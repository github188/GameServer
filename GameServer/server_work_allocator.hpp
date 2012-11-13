

#ifndef BAS_GAME_SERVER_WORK_ALLOCATOR_HPP
#define BAS_GAME_SERVER_WORK_ALLOCATOR_HPP

#include "server_work.hpp"

namespace GameServer {

class server_work_allocator
{
public:
  typedef boost::asio::ip::tcp::socket socket_type;

  server_work_allocator()
  {
  }

  socket_type* make_socket(boost::asio::io_service& io_service)
  {
    return new socket_type(io_service);
  }

  server_work* make_handler()
  {
    return new server_work();
  }
};

} // namespace GameServer

#endif // BAS_GAME_SERVER_WORK_ALLOCATOR_HPP
