#pragma once

//#define ENABLE_LOGGING

#include <iostream>
#include <sstream>
#include <string>

void commit_log_message(std::string &&);

#define _DO_LOG(msg) \
    do { \
        std::ostringstream __oss; \
        __oss << __FILE__ << "(" << __LINE__ << "): "<< msg << "\n"; \
        commit_log_message(std::move(__oss).str()); \
    } while (0)

#define ALWAYS_LOG(msg) _DO_LOG(msg)

#ifdef ENABLE_LOGGING
    #define LOG_LINE(msg) _DO_LOG(msg)
#else
    #define LOG_LINE(msg)
#endif