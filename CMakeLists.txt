cmake_minimum_required(VERSION 3.10)

project(SimpleHttpServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(SimpleHttpServer run_http_server.cpp)
