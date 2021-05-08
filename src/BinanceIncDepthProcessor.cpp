#include "BinanceIncDepthProcessor.h"

#include "Log.h"

#include <rapidjson/document.h>

namespace binance {

bool BinanceIncDepthProcessor::process(const std::string_view data)
{
    LOG_LINE("BinanceIncDepthProcessor::process()");

    rapidjson::Document d;
    d.Parse(data.data(), data.size());
    assert(d.IsObject());

    const auto ts = d["E"].GetInt64();
    const std::chrono::milliseconds ms_ts(ts);

    // Our delay cannot be more than seconds, therefore we do not need to convert
    // timezones, just need to remove minutes difference between us and binance server

    const std::chrono::system_clock::time_point event_ts(ms_ts);
    const auto now = std::chrono::system_clock::now();
    const auto diff = now - event_ts;
    const auto abs_diff = diff > std::chrono::microseconds(0) ? diff : -diff;
    const auto minutes_diff = std::chrono::duration_cast<std::chrono::minutes>(abs_diff);

    const auto latency = abs_diff - minutes_diff;

    std::unique_lock lock(m_mutex);

    m_stat.add_update(latency);

    if (d.HasMember("b")) {
        const auto bids = d["b"].GetArray();
        LOG_LINE("Bids size: " << bids.Size());
        for (const auto & b : bids) {
            const auto price_volume = b.GetArray();
            LOG_LINE("Price volume size: " << price_volume.Size());
            const auto price_str = price_volume[0].GetString();
            const auto volume_str = price_volume[1].GetString();
            LOG_LINE("Level: " << volume_str << "@" << price_str);
            m_order_book.insert_replace(std::stod(price_str), std::stod(volume_str), OrderBook::Side::Bid);
        }
    }
    if (d.HasMember("a")) {
        const auto asks = d["a"].GetArray();
        LOG_LINE("Asks size: " << asks.Size());
        for (const auto & a : asks) {
            const auto price_volume = a.GetArray();
            LOG_LINE("Price volume size: " << price_volume.Size());
            const auto price_str = price_volume[0].GetString();
            const auto volume_str = price_volume[1].GetString();
            LOG_LINE("Level: " << volume_str << "@" << price_str);
            m_order_book.insert_replace(std::stod(price_str), std::stod(volume_str), OrderBook::Side::Ask);
        }
    }

    return true;
}

void BinanceIncDepthProcessor::failure(const std::string_view reason)
{
    ALWAYS_LOG("BinanceIncDepthProcessor::failure(), reason: " << reason);

    std::unique_lock lock(m_mutex);
    m_stat.clear();
    m_order_book.clear();
}

Statistics BinanceIncDepthProcessor::get_statistics() const
{
    std::shared_lock lock(m_mutex);
    return m_stat;
}

OrderBook BinanceIncDepthProcessor::get_order_book() const
{
    std::shared_lock lock(m_mutex);
    return m_order_book;
}

}