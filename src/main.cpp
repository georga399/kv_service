#include "tcp_server.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <port> <connection_string>"
                << std::endl;
      return 1;
    }
    int port = std::atoi(argv[1]);
    if (port < 1 || port > 65535) {
      std::cerr << "Error: Port number must be between 1 and 65535."
                << std::endl;
      return 1;
    }
    std::string connection_string = argv[2];

    boost::asio::io_context io_context;
    tcp_server server(io_context, port, connection_string);
    std::vector<std::thread> threads;
    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
      threads.emplace_back([&io_context]() { io_context.run(); });
    }

    for (auto &thread : threads) {
      thread.join();
    }
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
