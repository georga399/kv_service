#pragma once
#include "common.hpp"

class tcp_server {
public:
    tcp_server(asio::io_context& io_context, short port, std::string connection_string);
private:
    void do_accept();
    tcp::acceptor m_acceptor;
    std::string m_connection_string;
};