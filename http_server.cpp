#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/system/error_code.hpp>
#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/interprocess/streams/bufferstream.hpp>
#include <set>
#include <map>
#include "server.hpp"
#include "connection.hpp"
#include "single_conn.hpp"
#include <sys/types.h>
#include <sys/wait.h>

using boost::asio::ip::tcp;
using namespace std;
void connection::start(std::shared_ptr<single_conn> c)
{
    connections_.insert(c);
    c->start();
}
void connection::stop(std::shared_ptr<single_conn> c)
{
    connections_.erase(c);
    c->stop();
}
void connection::stop_all(){
    for (auto &i : connections_) {
        i->stop();
    }
    connections_.clear();
}

void single_conn::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(
    boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, size_t length) {
            if (!ec) {
                std::size_t pos;
                string line;
                boost::interprocess::bufferstream input(data_, strlen(data_));
                getline(input, line, '\n');
                if (line.back() == '\r') {
                    line.resize(line.size()-1);
                }
                header["REQUEST_METHOD"] = line.substr(0, line.find(" "));
                line = line.substr(line.find("/"));
                if ((pos = line.find(" ")) != string::npos)
                {
                    header["REQUEST_URI"] = line.substr(line.find("/"), pos);
                    if ((pos = line.find("?")) != string::npos)
                        header["URI"] = line.substr(line.find("/"), pos);
                    else
                        header["URI"] = header["REQUEST_URI"];
                    header["SERVER_PROTOCOL"] = line.substr(line.find("H"));
                }
                else
                    header["REQUEST_URI"] = line.substr(line.find("/"));
                        if ((pos = header["REQUEST_URI"].find("?")) != string::npos)
                {
                    auto pos2 = header["REQUEST_URI"].find(" ");
                    header["QUERY_STRING"] = header["REQUEST_URI"].substr(pos + 1, pos2);
                }
                else
                    header["QUERY_STRING"] = "";
                cout << "QUERY: "<< header["QUERY_STRING"] << endl;
                while (getline(input, line, '\n')) {
                    if (line.empty() || line == "\r") {
                        break; // end of headers reached
                    }
                    if (line.back() == '\r') {
                        line.resize(line.size()-1);
                    }
                    if ((pos = line.find("Host")) != std::string::npos)
                    {
                        header["HTTP_HOST"] = line.substr(6);
                    }
                }

                HandleRequest_(true);
                do_read();

            } else if (ec != boost::asio::error::operation_aborted) {
                cn_.stop(shared_from_this());
            }
	    else
		    cout << ec.message();
        });
}

void single_conn::HandleRequest_(bool is_good_request){
    auto self(shared_from_this());

    bool is_ok = false;
    std::string reply_msg;
    boost::filesystem::path execfile;
    if (is_good_request) {
        execfile = boost::filesystem::current_path() / header["URI"];
        cout << execfile << endl;
	if (boost::filesystem::is_regular_file(execfile)) {
        is_ok = true;
        reply_msg +=
            "HTTP/1.1 200 OK\r\n";
        } else {
        reply_msg +=
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 20\r\n"
            "\r\n"
            "<h1>Not Found</h1>\r\n";
        }
    } else {
        reply_msg +=
            "HTTP/1.0 400 Bad Request\r\n"
            "\r\n";
    }

    boost::asio::async_write(socket_, 
        boost::asio::buffer(reply_msg.c_str(), strlen(reply_msg.c_str())),
        [this, self, is_ok, execfile](boost::system::error_code ec,
                                        size_t length) {
                if (!ec && is_ok) {
                setenv("REQUEST_METHOD", header["REQUEST_METHOD"].c_str(), 1);
		cout << "REQUEST_METHOD" << header["REQUEST_METHOD"] << endl;
                setenv("REQUEST_URI", header["REQUEST_URI"].c_str(), 1);
		cout << "REQUEST_URI" << header["REQUEST_URI"] << endl;
                setenv("QUERY_STRING", header["QUERY_STRING"].c_str(), 1);
		cout << "QUERY_STRING" << header["QUERY_STRING"] << endl;
                setenv("SERVER_PROTOCOL",
                        header["SERVER_PROTOCOL"].c_str(),
			//boost::str(boost::format{"HTTP%1%.%2%"} % header["SERVER_PROTOCOL_1"]
                        //         % header["SERVER_PROTOCOL_2"]).c_str(),
                        1);
		cout <<"SERER_PROTOCOL" << header["SERVER_PROTOCOL"] << endl;
                setenv("HTTP_HOST", header["HTTP_HOST"].c_str(), 1);
		cout << "HTTP_HOST"<< header["HTTP_HOST"] << endl;
                setenv("SERVER_ADDR",
                        socket_.local_endpoint().address().to_string().c_str(), 1);
                cout << "SERVER_ADDR" << socket_.local_endpoint().address().to_string() << endl;
		setenv("SERVER_PORT",
                        boost::str(boost::format{"%1%"} % socket_.local_endpoint().port()).c_str(), 1);
                cout << "SERVER_PORT" << boost::str(boost::format{"%1%"} % socket_.local_endpoint().port()) << endl;
		setenv("REMOTE_ADDR",
                        socket_.remote_endpoint().address().to_string().c_str(), 1);
                setenv("REMOTE_PORT",
                        boost::str(boost::format{"%1%"} % socket_.remote_endpoint().port()).c_str(), 1);

                io_context_.notify_fork(boost::asio::io_context::fork_prepare);
                if (fork() != 0) {
                    io_context_.notify_fork(boost::asio::io_context::fork_parent);
                } else {
                    io_context_.notify_fork(boost::asio::io_context::fork_child);
                    int sockfd = socket_.native_handle();
                    dup2(sockfd, 0);
                    dup2(sockfd, 1);
                    dup2(sockfd, 2);
                    cn_.stop_all();

                    if (execlp(execfile.c_str(), execfile.c_str(), NULL) < 0) {
                        cout << "Content-type:text/html\r\n"
                                "\r\n"
                                "<h1>Execution failed</h1>\r\n";
                    }

                    boost::system::error_code ignored_ec;
                    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
                }
            }

            if (ec != boost::asio::error::operation_aborted) {
                cn_.stop(self);
            }
    });
}

void server::do_wait()
{
    signal_.async_wait(
    [this](const boost::system::error_code& ec, int sig) {
        if (acceptor_.is_open()) {
            int status = 0;
            while (waitpid(-1, &status, WNOHANG) > 0);
            do_wait();
        }
    });
}

void server::do_accept(boost::asio::io_context& io_context)
{
    acceptor_.async_accept(
        [this, &io_context](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
        {
            if (!ec)
            {
                cn_.start(make_shared<single_conn>(
                    move(socket), cn_, io_context));
            }
            do_accept(io_context);
    });
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;
    server s(io_context, std::atoi(argv[1]));
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
