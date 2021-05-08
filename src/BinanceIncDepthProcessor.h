#pragma once

#include "OrderBook.h"
#include "IJsonDataListener.h"

#include <shared_mutex>

namespace binance {

class BinanceIncDepthProcessor final
    : public IDepthDataListener
{
public:
    BinanceIncDepthProcessor(bool build_order_book);

    bool process(std::string_view data) final;
    void failure(std::string_view reason) final;

    Statistics get_statistics() const final;
    OrderBook get_order_book() const final;

private:
    mutable std::shared_mutex m_mutex;

    const bool m_build_order_book;
    OrderBook m_order_book;
    Statistics m_stat;
};

}
