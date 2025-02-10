FROM ubuntu:24.04

ARG BOOST_VERSION=1.87.0
ARG CMAKE_VERSION=3.31.4
ARG NUM_JOBS=8


ENV DEBIAN_FRONTEND=noninteractive

# Install necessary packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        software-properties-common \
        autoconf \
        automake \
        libtool \
        pkg-config \
        ca-certificates \
        libssl-dev \
        wget \
        git \
        curl \
        language-pack-en \
        locales \
        locales-all \
        vim \
        gdb \
        valgrind && \
    apt-get clean

# System locale
# Important for UTF-8
ENV LC_ALL=en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US.UTF-8

# Install CMake
RUN cd /tmp && \
    wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz && \
    tar xzf cmake-${CMAKE_VERSION}.tar.gz && \
    cd cmake-${CMAKE_VERSION} && \
    ./bootstrap && \
    make -j${NUM_JOBS} && \
    make install && \
    rm -rf /tmp/*

# Install Boost
# https://www.boost.org/doc/libs/1_80_0/more/getting_started/unix-variants.html
RUN cd /tmp && \
    BOOST_VERSION_MOD=$(echo $BOOST_VERSION | tr . _) && \
    wget https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_MOD}.tar.bz2 && \
    tar --bzip2 -xf boost_${BOOST_VERSION_MOD}.tar.bz2 && \
    cd boost_${BOOST_VERSION_MOD} && \
    ./bootstrap.sh --prefix=/usr/local && \
    ./b2 install && \
    rm -rf /tmp/*

# # Install Google Test
RUN apt-get update && apt-get install -y \
    libgtest-dev 
#     && cd /usr/src/gtest && \
#     cmake . && \
#     make && \
#     cp libgtest*.a /usr/lib/ && \
#     cp libgtest_main.a /usr/lib/

RUN apt-get update && apt-get install -y libpqxx-dev libpq-dev

# Set the working directory
WORKDIR /app

# Copy the source code into the container
COPY . .

# Initialize and update git submodules
RUN git submodule update --init --recursive

# Build the application using CMake
RUN mkdir build && cd build && cmake .. && make -j

# Set the entry point for the container
ENTRYPOINT ["./build/server"]
# ENTRYPOINT ["./build/server", "5003", "postgresql://postgres:changeme@postgres:5432/postgres"]

# Expose the port (replace 5003 with your desired port)
EXPOSE 5003