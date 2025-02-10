# Key-Value Storage Server

This project implements a simple key-value storage server in C++ using Boost.Asio for asynchronous TCP network communication and PostgreSQL for persistent storage. The server handles concurrent client connections, allowing them to read and write string key-value pairs.  Data persists across server restarts.

## Technologies

*   **C++:** Core implementation language.
*   **Boost.Asio:** Asynchronous networking library.
*   **nlohmann/json:** JSON parsing and serialization library.
*   **PostgreSQL:** Database for persistent storage.
*   **libpqxx** Library for PostgreSQL.
*   **GTest:** Testing framework.
*   **CMake:** Build system.

## Building the Project

### Prerequisites

*   C++ compiler (e.g., g++, clang++) with C++17 support
*   Boost library (version 1.70 or higher)
*   PostgreSQL development libraries
*   CMake (version 3.15 or higher)
*   nlohmann/json library (included in the project)
*   GTest library (included in the project)

### Build locally Instructions

1.  **Clone the repository:**

    ```
    git clone https://github.com/georga399/kv_service
    cd kv_service
    ```
2. **Pull third party:**

    ```
    git submodule update --init --recursive
    ```

3.  **Create a build directory:**

    ```
    mkdir build
    cd build
    ```


4.  **Configure the project with CMake:**

    ```
    cmake ..
    ```

5.  **Build the project:**

    ```
    make -j$(nproc)
    ```

## Running the Server

### Run server locally

```
./server <port number> <connection string to the postgresql db>
```

## Run server with Docker

```
docker compose up
```
### Example Client Interaction (using `netcat`)

1.  **Write a key-value pair:**

    ```
    echo -n '{"request": "write", "key": "mykey", "value": "myvalue"}' | nc localhost 5003
    ```

2.  **Read the key-value pair:**

    ```
    echo -n '{"request": "read", "key": "mykey"}' | nc localhost 5003
    ```

    *   The server will respond with:

        ```
        {"status": "ok", "value": "myvalue"}
        ```

3.  **Read a non-existent key:**

    ```
    echo -n '{"request": "read", "key": "nonexistent"}' | nc localhost 5003
    ```

    *   The server will respond with an error:

        ```
        {"status": "error", "description": "Key not found"}
        ```


## Using the Server

The server listens for incoming TCP connections on the specified port. Clients can send JSON requests to the server to read or write key-value pairs.

### Request Format

All requests must be null-terminated JSON strings.

#### Read Request

```
{"request": "read", "key": "some_key"}
```

#### Write Request

```
{"request": "write", "key": "some_key", "value": "some_value"}
```

### Response Format

All responses are null-terminated JSON strings.

#### Successful Read Response

```
{"status": "ok", "value": "some_value"}
```

#### Successful Write Response

```
{"status": "ok"}
```

#### Error Response

```
{"status": "error", "description": "error description"}
```


## Running Tests

To run the server-side tests inside build/ directory:

```
ctest
```

