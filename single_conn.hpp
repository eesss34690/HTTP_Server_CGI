#ifndef _SINGLE_CONN_HPP_
#define _SINGLE_CONN_HPP_
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <map>
#include <string>
#include "connection.hpp"
using namespace std;
using boost::asio::ip::tcp;

class connection;
class single_conn
 : public enable_shared_from_this<single_conn> {
private:
    tcp::socket socket_;
    enum { max_length = 15000 };
    char data_[max_length];
    map<string, string> header;
    connection& cn_;
    boost::asio::io_context& io_context_;
    void do_read();
    void HandleRequest_(bool is_good_request = true);

public:
    single_conn(tcp::socket socket,
             connection& cn,
             boost::asio::io_context& io_context)
      : socket_(move(socket)),
        cn_(cn),
        io_context_(io_context){};

    void stop() { socket_.close(); }
    void start() { do_read(); };
};
#endif