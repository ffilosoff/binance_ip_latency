#pragma once

#include "IJsonDataListener.h"
#include "IConnector.h"
#include "IPAddress.h"

#include <memory>

namespace binance {

class BinanceWebSocketConnector final
    : public IConnector
{
    class Impl;
public:
    explicit BinanceWebSocketConnector(const IPAddress &, const Port &, std::string request, JsonDataListenerPtr listener = {});
    ~BinanceWebSocketConnector() final;

    void start() final;
    void stop() final;

    bool is_running() const final;

    IPAddress get_host() const final;

    static std::unique_ptr<BinanceWebSocketConnector> make_depth_connector(const IPAddress &, const Port &, std::string ticker, JsonDataListenerPtr listener = {});

private:
    std::unique_ptr<Impl> m_impl;
};

}