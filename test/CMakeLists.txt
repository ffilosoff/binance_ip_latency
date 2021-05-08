cmake_minimum_required(VERSION 3.13)
project(binance_ip_lookup_test)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest)

if (GTest_FOUND)
add_executable(
        binance_ip_lookup_test
        ../src/OrderBook.cpp
        OrderBookTest.cpp
)

target_link_libraries(${PROJECT_NAME} ${GTEST_BOTH_LIBRARIES})

find_package(Threads)
if (Threads_FOUND)
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif()

add_test(${PROJECT_NAME} binance_ip_lookup_test)
endif()
