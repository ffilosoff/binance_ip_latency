#include "Log.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>

template<class Clock, class Duration>
struct TimePrinter {
    explicit constexpr TimePrinter(const std::chrono::time_point<Clock, Duration> & time)
        : m_time(time)
    { } 

    friend std::ostream & operator<<(std::ostream & oss, const TimePrinter<Clock, Duration> & time) { time.print(oss); return oss; }
private:
    void print(std::ostream & oss) const
    {   
        const time_t time = Clock::to_time_t(m_time);
        auto tm = std::localtime(&time);
        oss << std::put_time(tm, "%F %T");
    }   

private:
    const std::chrono::time_point<Clock> & m_time;
};

static std::mutex log_mutex;

void commit_log_message(std::string && str)
{
    const auto now = std::chrono::system_clock::now();
    // simple protection for mutlithreaded output, can be improved with any async logging
    std::lock_guard lock(log_mutex);
    std::cout << TimePrinter(now) << ": " << str;
}
