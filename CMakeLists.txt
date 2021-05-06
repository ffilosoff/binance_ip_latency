cmake_minimum_required(VERSION 3.13)

project(binance_ip_lookup)

set (CMAKE_CXX_STANDARD 17)

add_executable(binance_ip_lookup main.cpp DNSLookup.cpp)

target_link_libraries(binance_ip_lookup resolv)