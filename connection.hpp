#ifndef _CONNECTION_HPP_
#define _CONNECTION_HPP_

#include <memory>
#include <utility>
#include <set>
#include "single_conn.hpp"
#include <iostream>

using namespace std;
class single_conn;
class connection {
private:
    set<shared_ptr<single_conn>> connections_;
public:
    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    connection() {}

    void start(shared_ptr<single_conn> c);
    void stop(shared_ptr<single_conn> c);
    void stop_all();
};

#endif
