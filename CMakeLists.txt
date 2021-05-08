cmake_minimum_required(VERSION 3.13)

project(binance_ip_lookup)

set (CMAKE_CXX_STANDARD 17)

add_executable(binance_ip_lookup
        src/main.cpp
        src/DNSLookup.cpp
        src/BinanceWebSocketConnector.cpp
        src/OrderBook.cpp
        src/Log.cpp
        src/BinanceIncDepthProcessor.cpp)

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()

find_package(OpenSSL REQUIRED)
if (OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
endif()

find_package(RapidJSON REQUIRED)
if (RapidJSON_FOUND)
    include_directories(${RapidJSON_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${RapidJSON_LIBRARIES})
endif()

FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Boost_LIBRARIES})

find_package(Threads)
if (Threads_FOUND)
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif()

target_link_libraries(binance_ip_lookup resolv)

add_subdirectory(test)
