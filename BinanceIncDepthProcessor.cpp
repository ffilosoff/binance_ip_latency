#include "BinanceIncDepthProcessor.h"

#include "Log.h"

#include <rapidjson/document.h>

bool binance::BinanceIncDepthProcessor::process(const std::string_view data)
{
    LOG_LINE("BinanceIncDepthProcessor::process()");

    rapidjson::Document d;
    d.Parse(data.data(), data.size());
    assert(d.IsObject());

    const auto ts = d["E"].GetInt64();
    const std::chrono::milliseconds ms_ts(ts);

    const std::chrono::system_clock::time_point event_ts(ms_ts);
    const std::chrono::milliseconds after_ms = std::chrono::duration_cast<std::chrono::milliseconds>(event_ts.time_since_epoch()); // tmp
    const auto now = std::chrono::system_clock::now();
    const std::chrono::milliseconds sys_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()); // tmp
    LOG_LINE("Was ts [" << ts << "], after converstions [ " << after_ms.count() << "], system ts: [" << sys_ms.count() << "]");
    const auto diff = now - event_ts;
    const auto abs_diff = diff > std::chrono::microseconds(0) ? diff : -diff;
    const auto minutes_diff = std::chrono::duration_cast<std::chrono::minutes>(abs_diff);

    const auto latency = abs_diff - minutes_diff;

    std::unique_lock lock(m_mutex);

    m_stat.add_update(latency);

    // TODO: update depth

    return true;
}

Statistics binance::BinanceIncDepthProcessor::get_statistics() const
{
    std::shared_lock lock(m_mutex);
    return m_stat;
}
