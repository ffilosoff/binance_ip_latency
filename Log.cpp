#include "Log.h"

#include <iostream>
#include <mutex>

static std::mutex log_mutex;

void commit_log_message(std::string && str)
{
    // simple protection for mutlithreaded output, can be improved with any async logging
    std::lock_guard lock(log_mutex);
    std::cout << str;
}