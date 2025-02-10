#pragma once
#include "common.hpp"
#include "storage.hpp"
#include <memory>

class session : public std::enable_shared_from_this<session> {
public:
  session(tcp::socket socket, std::string connection_string);
  void run();

private:
  void wait_for_request();
  void process_request(const std::string &);
  void send_response(const std::string &);

  tcp::socket m_socket;
  storage m_storage;
  boost::asio::streambuf m_buffer;
  std::string m_connection_string;
};