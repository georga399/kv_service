#include "storage.hpp"

storage::storage(std::string connection_string) {
  conn_ = std::make_unique<pqxx::connection>(connection_string);
  pqxx::work txn(*conn_);
  txn.exec("CREATE TABLE IF NOT EXISTS key_value_store (key TEXT PRIMARY KEY, "
           "value TEXT)");
  txn.commit();
}

void storage::write(const std::string &key, const std::string &value) {
  pqxx::work txn(*conn_);
  txn.exec("INSERT INTO key_value_store (key, value) VALUES ($1, $2) "
                  "ON CONFLICT (key) DO UPDATE SET value = $2", pqxx::params(key, value));
  txn.commit();
}

std::string storage::read(const std::string &key) {
  pqxx::work txn(*conn_);
  auto result = txn.exec(
      "SELECT value FROM key_value_store WHERE key = $1", pqxx::params(key));
  if (result.empty()) {
    throw std::runtime_error("Key not found");
  }
  return result[0][0].as<std::string>();
}