#include "../src/storage.hpp"
#include "../src/tcp_server.hpp"
#include <boost/asio.hpp>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

using namespace boost::asio;
using namespace ip;
using json = nlohmann::json;

class ServerIntegrationTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Clear database
    pqxx::connection conn(
        "postgresql://postgres:changeme@localhost:5432/postgres");
    pqxx::work txn(conn);
    txn.exec("TRUNCATE TABLE key_value_store");
    txn.commit();

    // Start server with correct io_context type
    io_context = std::make_unique<boost::asio::io_context>();
    server = std::make_unique<tcp_server>(
        *io_context, 54321,
        "postgresql://postgres:changeme@localhost:5432/postgres");
    server_thread = std::thread([this]() { io_context->run(); });
  }

  void TearDown() override {
    io_context->stop();
    if (server_thread.joinable()) {
      server_thread.join();
    }
  }

  std::unique_ptr<boost::asio::io_context> io_context;
  std::unique_ptr<tcp_server> server;
  std::thread server_thread;

  std::string send_request(const std::string &request) {
    boost::asio::io_context client_io;
    tcp::socket socket(client_io);
    tcp::resolver resolver(client_io);

    connect(socket, resolver.resolve("127.0.0.1", "54321"));

    // Send request with null terminator
    std::string full_request = request + '\0';
    boost::asio::write(socket, boost::asio::buffer(full_request));

    // Read response
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\0");

    std::string data(boost::asio::buffers_begin(response.data()),
                     boost::asio::buffers_begin(response.data()) +
                         response.size());
    return data;
  }
};

TEST_F(ServerIntegrationTest, WriteAndReadValidKey) {
  // Test write operation
  json write_response = json::parse(send_request(
      R"({"request":"write","key":"test_key","value":"test_value"})"));
  EXPECT_EQ(write_response["status"], "ok");

  // Test read operation
  json read_response =
      json::parse(send_request(R"({"request":"read","key":"test_key"})"));
  EXPECT_EQ(read_response["status"], "ok");
  EXPECT_EQ(read_response["value"], "test_value");
}

TEST_F(ServerIntegrationTest, ConcurrentWrites) {
  constexpr int THREAD_COUNT = 10;
  std::vector<std::thread> threads;
  std::atomic<int> success_count(0);

  auto client_task = [&](int id) {
    try {
      boost::asio::io_context client_io;
      tcp::socket socket(client_io);
      tcp::resolver resolver(client_io);
      connect(socket, resolver.resolve("127.0.0.1", "54321"));

      json req = {{"request", "write"},
                  {"key", "concurrent_key"},
                  {"value", "thread_" + std::to_string(id)}};

      std::string request = req.dump() + '\0';
      boost::asio::write(socket, boost::asio::buffer(request));

      boost::asio::streambuf response;
      boost::asio::read_until(socket, response, "\0");
      json res = json::parse(std::string(
          boost::asio::buffers_begin(response.data()),
          boost::asio::buffers_begin(response.data()) + response.size()));

      if (res["status"] == "ok")
        success_count++;
    } catch (...) {
    }
  };

  for (int i = 0; i < THREAD_COUNT; ++i) {
    threads.emplace_back(client_task, i);
  }

  for (auto &t : threads)
    t.join();

  // Verify at least one write succeeded
  json read_res =
      json::parse(send_request(R"({"request":"read","key":"concurrent_key"})"));

  EXPECT_GE(success_count, 1);
  if (read_res["status"] == "ok") {
    EXPECT_TRUE(read_res["value"].get<std::string>().starts_with("thread_"));
  }
}

TEST_F(ServerIntegrationTest, MalformedRequest) {
  // Missing key field
  json response =
      json::parse(send_request(R"({"request":"write","value":"test"})"));
  EXPECT_EQ(response["status"], "error");

  // Invalid value type
  response = json::parse(
      send_request(R"({"request":"write","key":"test","value":123})"));
  EXPECT_EQ(response["status"], "error");
}