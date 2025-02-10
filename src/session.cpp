#include "session.hpp"
#include <boost/asio/write.hpp>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

session::session(tcp::socket socket, std::string connection_string)
    : m_socket(std::move(socket)), m_storage(connection_string),
      m_connection_string(connection_string) {}

void session::run() { wait_for_request(); }

void session::wait_for_request() {
  auto self(shared_from_this());
  boost::asio::async_read_until(
      m_socket, m_buffer, "\0",
      [this, self](boost::system::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          std::string data{std::istreambuf_iterator<char>(&m_buffer),
                           std::istreambuf_iterator<char>()};
          std::cout << data << std::endl;
          process_request(data);
        } else {
          std::cout << "error: " << ec << std::endl;
        }
      });
}

void session::process_request(const std::string &request) {
  json response;
  try {
    json req = json::parse(request);
    std::string request_type = req["request"];
    std::string key = req["key"];
    if (request_type == "read") {
      std::string value = m_storage.read(key);
      response = {{"status", "ok"}, {"value", value}};
    } else if (request_type == "write") {
      std::string value = req["value"];
      m_storage.write(key, value);
      response = {{"status", "ok"}};
    } else {
      response = {{"status", "error"}, {"description", "Invalid request type"}};
    }
  } catch (const std::exception &e) {
    response = {{"status", "error"}, {"description", e.what()}};
  }
  std::cout << "proccess_request:" << response.dump() << std::endl;
  send_response(response.dump());
}

void session::send_response(const std::string &response) {
  std::cout << response;
  auto self(shared_from_this());
  asio::async_write(m_socket, asio::buffer(response),
                    [this, self](boost::system::error_code ec, std::size_t) {
                      if (!ec) {
                        m_socket.close();
                      }
                    });
}