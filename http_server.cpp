#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class sigle_conn;
class connect {
 private:
  set<shared_ptr<sigle_conn>> connections_;

 public:
  connect(const connect&) = delete;
  connect& operator=(const connect&) = delete;

  connect() {}

  void start(shared_ptr<sigle_conn> c)
  {
      connections_.insert(c);
      c->start();
  }
  void stop(shared_ptr<sigle_conn> c)
  {
      connections_.erase(c);
      c->stop();
  }
  void stop_all(){
    for (auto &i : connections_) {
        i->stop();
    }
    connections_.clear();
  }
};

class sigle_conn
 : public enable_shared_from_this<sigle_conn> {
private:
  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length], send[max_length];
  std::map<string, string> header;
  connect& cn_;
  boost::asio::io_context& io_context_;
  void do_read()
  {
      auto self(shared_from_this());
      std::string line;
      socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this, self](system::error_code ec, size_t length) {
            if (!ec) {
                std::size_t pos;
                getline(buffer, line, '\n');
                if (line.empty() || line == "\r") {
                    break; // end of headers reached
                }
                if (line.back() == '\r') {
                    line.resize(line.size()-1);
                }
                header["REQUEST_METHOD"] = line.substr(0, line.find(" "));
                header["REQUEST_URI"] = line.substr(line.find("/"), line.find("?"));
                header["QUERY_STRING"] = line.substr(line.find("?"));
                while (getline(buffer, line, '\n')) {
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
                    else if ((pos = line.find("User-Agent")) != std::string::npos)
                    {
                        pos = line.find("/")
                        auto pos2 = line.find(".")
                        header["SERVER_PROTOCOL_1"] = line.substr(pos, pos2);
                        header["SERVER_PROTOCOL_2"] = line.substr(pos2);
                    }
                }

                HandleRequest_();
                do_read();

            } else if (ec != asio::error::operation_aborted) {
                cn_.stop(shared_from_this());
            }
      });
  }
  void HandleRequest_(bool is_good_request = true){
    auto self(shared_from_this());

    bool is_ok = false;
    string reply_msg;
    boost::filesystem::path execfile;
    if (is_good_request) {
        execfile = boost::filesystem::current_path() / header["REQUEST_URI"];
        if (filesystem::is_regular_file(execfile)) {
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
        boost::asio::buffer(reply_msg.c_str(), lenof reply_msg),
        [this, self, is_ok, execfile](system::error_code ec,
                                        size_t length) {
                if (!ec && is_ok) {
                setenv("REQUEST_METHOD", header["REQUEST_METHOD"].c_str(), 1);
                setenv("REQUEST_URI", header["REQUEST_URI"].c_str(), 1);
                setenv("QUERY_STRING", header["QUERY_STRING"].c_str(), 1);
                setenv("SERVER_PROTOCOL",
                        fmt::format("HTTP{}.{}", stoi(header["SERVER_PROTOCOL_1"]),
                                stoi(header["SERVER_PROTOCOL_2"])).c_str(),
                        1);
                setenv("HTTP_HOST", header["HTTP_HOST"].c_str(), 1);
                setenv("SERVER_ADDR",
                        socket_.local_endpoint().address().to_string().c_str(), 1);
                setenv("SERVER_PORT",
                        fmt::format("{}", socket_.local_endpoint().port()).c_str(), 1);
                setenv("REMOTE_ADDR",
                        socket_.remote_endpoint().address().to_string().c_str(), 1);
                setenv("REMOTE_PORT",
                        fmt::format("{}", socket_.remote_endpoint().port()).c_str(),
                        1);

                io_context_.notify_fork(asio::io_context::fork_prepare);
                if (fork() != 0) {
                    io_context_.notify_fork(asio::io_context::fork_parent);
                } else {
                    io_context_.notify_fork(asio::io_context::fork_child);
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

                    system::error_code ignored_ec;
                    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
                }
            }

            if (ec != asio::error::operation_aborted) {
                cn_.stop(self);
            }
    });
  }


public:
  sigle_conn(boost::asio::ip::tcp::socket socket,
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
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    : io_context_(io_context)
  {
    do_wait();
    do_accept();
  }

private:
  void do_wait()
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
  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            cn_.start(make_shared<Connection>(
                move(socket), cn_, io_context_));
          }
          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  boost::asio::signal_set signal_;
  boost::asio::io_context io_context_;
  connect cn_;
  
};

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