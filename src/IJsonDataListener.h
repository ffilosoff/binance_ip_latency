#pragma once

#include "OrderBook.h"

#include <memory>
#include <ostream>
#include <string_view>

struct Statistics
{
    void add_update(std::chrono::microseconds diff)
    {
        if (min_time > diff) {
            min_time = diff;
        }
        if (max_time < diff) {
            max_time = diff;
        }
        sum_time += diff;
        ++num_updates;
    }

    void clear()
    {
        *this = Statistics();
    }

    auto get_min_time() const { return min_time; }
    auto get_max_time() const { return max_time; }
    auto get_avg_time() const { return num_updates ? std::chrono::microseconds(sum_time / num_updates) : sum_time; }

    bool empty() const { return num_updates != 0; }

    std::ostream & print(std::ostream & strm) const
    {
        if (num_updates) {
            return strm << "min: " << min_time.count() << "us, max: " << max_time.count() << "us, avg: "
                        << (sum_time / num_updates).count() << "us";
        } else {
            return strm << "<empty>";
        }
    }

    friend std::ostream & operator<< (std::ostream & strm, const Statistics & s) { return s.print(strm); }

private:
    std::chrono::microseconds min_time{std::numeric_limits<decltype(std::chrono::microseconds().count())>::max()};
    std::chrono::microseconds max_time{std::numeric_limits<decltype(std::chrono::microseconds().count())>::min()};

    std::chrono::microseconds sum_time{0};
    size_t num_updates{0};
};

class IJsonDataListener
{
public:
    virtual ~IJsonDataListener() = default;

    virtual bool process(std::string_view data) = 0;
    virtual void failure(std::string_view reason) = 0;

    virtual Statistics get_statistics() const = 0;
};

class IDepthDataListener
    : public IJsonDataListener
{
public:
    virtual OrderBook get_order_book() const = 0;
};

using JsonDataListenerPtr = std::shared_ptr<IJsonDataListener>;
using DepthDataListenerPtr = std::shared_ptr<IDepthDataListener>;