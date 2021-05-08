#pragma once

#include "OrderBook.h"
#include "IJsonDataListener.h"

#include <shared_mutex>

namespace binance {

class BinanceIncDepthProcessor final
    : public IJsonDataListener
{
public:
    bool process(std::string_view data) final;

    Statistics get_statistics() const final;

private:
    mutable std::shared_mutex m_mutex;

    OrderBook m_order_book;
    Statistics m_stat;
};

}