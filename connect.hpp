#include <memory>
#include <set>

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