#include "../src/storage.hpp"
#include <gtest/gtest.h>
#include <pqxx/pqxx>

class StorageTest : public ::testing::Test {
protected:
  void SetUp() override {
    pqxx::connection conn(
        "postgresql://postgres:changeme@localhost:5432/postgres");
    pqxx::work txn(conn);
    txn.exec("TRUNCATE TABLE key_value_store");
    txn.commit();
  }
};
TEST_F(StorageTest, WriteThenReadSucceeds) {
  storage s("postgresql://postgres:changeme@localhost:5432/postgres");
  s.write("username", "user123");
  EXPECT_EQ(s.read("username"), "user123");
}

TEST_F(StorageTest, ReadNonExistentKeyThrows) {
  storage s("postgresql://postgres:changeme@localhost:5432/postgres");
  EXPECT_THROW(s.read("non_existent_key"), std::runtime_error);
}

TEST_F(StorageTest, OverwriteKeyUpdatesValue) {
  storage s("postgresql://postgres:changeme@localhost:5432/postgres");
  s.write("temp", "old_value");
  s.write("temp", "new_value");
  EXPECT_EQ(s.read("temp"), "new_value");
}