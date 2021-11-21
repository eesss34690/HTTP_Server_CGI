#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <set>
#include <map>
#include <string>

using namespace std;

class single_conn;
class connect {
private:
    set<shared_ptr<single_conn>> connections_;
public:
    connect(const connect&) = delete;
    connect& operator=(const connect&) = delete;

    connect() {}

    void start(shared_ptr<single_conn> c);
    void stop(shared_ptr<single_conn> c);
    void stop_all();
};

class single_conn
 : public enable_shared_from_this<single_conn> {
private:
    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length], send[max_length];
    map<string, string> header;
    connect& cn_;
    boost::asio::io_context& io_context_;
    void do_read();
    void HandleRequest_(bool is_good_request);

public:
    single_conn(boost::asio::ip::tcp::socket socket,
             connect& cn,
             boost::asio::io_context& io_context)
      : socket_(move(socket)),
        cn_(cn),
        io_context_(io_context){};

    void stop() { socket_.close(); }
    void start() { do_read(); };
};

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
    connect cn_;
  
};