#include <string> 
#include <pqxx/pqxx>

class storage { 
public: 
    storage(std::string connection_string); 
    void write(const std::string& key, const std::string& value); 
    std::string read(const std::string& key); 
private: 
    std::unique_ptr<pqxx::connection> conn_; 
    std::string conn_string;
};

