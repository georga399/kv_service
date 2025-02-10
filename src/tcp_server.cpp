#include "tcp_server.hpp"
#include "session.hpp"

tcp_server::tcp_server(asio::io_context &io_context, short port,
                       std::string connection_string)
    : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
      m_connection_string(connection_string) {
  do_accept();
}

void tcp_server::do_accept() {
  m_acceptor.async_accept(
      [this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
          std::cout << "creating session on: "
                    << socket.remote_endpoint().address().to_string() << ":"
                    << socket.remote_endpoint().port() << '\n';
          std::make_shared<session>(std::move(socket), m_connection_string)->run();
        } else {
          std::cout << "error: " << ec.message() << std::endl;
        }
        do_accept();
      });
}
