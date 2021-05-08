cmake_minimum_required(VERSION 3.13)
project(binance_ip_lookup_test)

set(CMAKE_CXX_STANDARD 17)

enable_testing()
find_package(GTest REQUIRED)

if (GTestFound)
add_executable(
        binance_ip_lookup_test
        ../src/OrderBook.cpp
        OrderBookTest.cpp
)

target_link_libraries(binance_ip_lookup_test GTest::gtest GTest::gtest_main)

add_test(AllTestsInFoo binance_ip_lookup_test)
endif()
