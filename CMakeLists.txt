cmake_minimum_required(VERSION 3.12)
project(kv_service CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_DEBUG ON)

find_package(Boost 1.87 REQUIRED COMPONENTS system thread)
find_package(GTest REQUIRED) 


include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# JSON library
set(JSON_BuildTests OFF CACHE INTERNAL "")


add_subdirectory(third_party/json)
add_subdirectory(third_party/libpqxx)

add_executable(server src/main.cpp
                    src/common.hpp
                    src/tcp_server.hpp
                    src/tcp_server.cpp
                    src/session.hpp
                    src/session.cpp
                    src/storage.hpp
                    src/storage.cpp
                    )

target_link_libraries(server 
                        ${Boost_LIBRARIES} 
                        nlohmann_json::nlohmann_json
                        pqxx)

target_compile_definitions(server PRIVATE BOOST_ASIO_NO_DEPRECATED)

enable_testing()

add_executable(storage_test
    tests/storage_test.cpp   
    src/storage.cpp          
    src/storage.hpp
)

add_executable(integration_tests
    tests/integration_tests.cpp
    src/tcp_server.cpp
    src/session.cpp
    src/storage.cpp
)

target_link_libraries(integration_tests
    PRIVATE 
    Boost::system
    Boost::thread
    pqxx
    nlohmann_json::nlohmann_json
    GTest::GTest 
    GTest::Main
)

target_link_libraries(storage_test
    PRIVATE
    GTest::GTest          
    GTest::Main          
    pqxx                 
)
include(GoogleTest)      
gtest_discover_tests(storage_test) 
gtest_discover_tests(integration_tests)