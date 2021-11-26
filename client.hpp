#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace boost::asio;

class client : enable_shared_from_this<client> {
private:
    string host_;
    string port_;
    string file_;
    string session;

    vector<string> cmd_list;
    int idx;

    enum { max_length = 1024 };
    char data_[max_length];

    boost::asio::ip::tcp::socket socket;
    boost::asio::ip::tcp::resolver resolver;
    void do_read();
    void do_write();
public:
    client(boost::asio::io_context& io_context, int id, string h, string p, string f)
        : host_(h), port_(p), file_(f), idx(0), session("s" + h[1]), socket(io_context), resolver(io_context){};
    void start();

};