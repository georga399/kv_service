#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

void print_usage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "For read:  ./client read <key>" << std::endl;
    std::cout << "For write: ./client write <key> <value>" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage();
        return 1;
    }

    std::string request_type = argv[1];
    std::string key = argv[2];
    json request;

    if (request_type == "read") {
        if (argc != 3) {
            print_usage();
            return 1;
        }
        request = {
            {"request", "read"},
            {"key", key}
        };
    } else if (request_type == "write") {
        if (argc != 4) {
            print_usage();
            return 1;
        }
        std::string value = argv[3];
        request = {
            {"request", "write"},
            {"key", key},
            {"value", value}
        };
    } else {
        std::cout << "Invalid request type. Use 'read' or 'write'." << std::endl;
        print_usage();
        return 1;
    }

    using boost::asio::ip::tcp;
    boost::asio::io_context io_context;

    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);

    try {
        boost::asio::connect(socket, resolver.resolve("127.0.0.1", "5003"));

        std::string data = request.dump() + '\0';
        auto result = boost::asio::write(socket, boost::asio::buffer(data));

        std::cout << "Data sent: " << data.length() << '/' << result << " bytes" << std::endl;

        boost::asio::streambuf response_buffer;
        boost::asio::read_until(socket, response_buffer, "\0");

        std::string response{
            boost::asio::buffers_begin(response_buffer.data()),
            boost::asio::buffers_end(response_buffer.data())  // Exclude null character
        };

        std::cout << "Response received: " << response << std::endl;

        boost::system::error_code ec;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket.close();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
