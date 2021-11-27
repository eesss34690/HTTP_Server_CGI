#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread/mutex.hpp>
#include <memory>
#include <string>
#include <vector>
#include "client_set.hpp"

using namespace std;
using namespace boost::asio;

class client_set;
class client
 : public enable_shared_from_this<client> {
private:
    string host_;
    string port_;
    string file_;
    string session;
    client_set& cs_;

    vector<string> cmd_list;
    int idx;

    enum { max_length = 1024 };
    char data_[max_length];

    boost::asio::ip::tcp::socket socket;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::io_context& io_context_;

    boost::mutex mtx_r;
    boost::mutex mtx_w;
    void do_read();
    void do_write();
public:
    client(boost::asio::io_context& io_context, client_set& cs, string s, string h, string p, string f);
    void start();

};
