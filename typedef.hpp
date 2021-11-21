#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <map>
#include <string>
#include <algorithm>
#include "single_conn.hpp"
#include "connection.hpp"

using namespace std;

class server
{
public:
    server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          io_context_(io_context)
    {
        do_wait();
        do_accept();
    }

private:
    void do_wait();
    void do_accept();

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::signal_set signal_;
    boost::asio::io_context io_context_;
    connection cn_;
  
};
#endif
