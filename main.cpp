#include <iostream>

#include "BinanceIncDepthProcessor.h"
#include "BinanceWebSocketConnector.h"
#include "DNSLookup.h"
#include "Helpers.h"
#include "Log.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <tuple>

int main() {
    const auto dns_lookup = create_dns_resolver();

    constexpr auto domain = "stream.binance.com";
    constexpr Port port = 9443;
    constexpr auto delay = std::chrono::seconds(2);
    constexpr auto ticker = "BTCUSDT";

    const auto ips = dns_lookup->resolve(domain);

    LOG_LINE("Resolved IPs [" << ips.size() << "]:\n" << SequencePrinter(ips, "\n"));

    std::vector<std::pair<std::unique_ptr<IConnector>, JsonDataListenerPtr>> measurers;
    measurers.reserve(ips.size());
    for (const auto & ip : ips) {
        auto listener = std::make_shared<binance::BinanceIncDepthProcessor>();
        auto copy_listener = listener;
        auto & it = measurers.emplace_back(binance::BinanceWebSocketConnector::make_depth_connector(ip, port, ticker, std::move(listener)), std::move(copy_listener));
        try {
            it.first->start();
        } catch (const std::exception & e) {
            LOG_LINE("Exception on starting listening to ip [" << ip << "]");
        }
    }

    std::vector<std::pair<std::string, Statistics>> stats;
    stats.reserve(measurers.size());
    while (true) {
        std::this_thread::sleep_for(delay); // TODO: replace with conditional wait_for()
        stats.clear();
        for (const auto & [connection, listener] : measurers) {
            stats.emplace_back(connection->get_host(), listener->get_statistics());
        }
        std::sort(stats.begin(), stats.end(), [] (const auto & a, const auto & b) { return a.second.get_avg_time() < b.second.get_avg_time(); });
        std::ostringstream oss;
        oss << "Statistics:\n";
        for (const auto & [host, s] : stats) {
            oss << std::setw(15) << host << ": " << s << "\n";
        }
        ALWAYS_LOG(oss.str());
    }
    return 0;
}