#include <iostream>

#include "BinanceIncDepthProcessor.h"
#include "BinanceWebSocketConnector.h"
#include "DNSLookup.h"
#include "Helpers.h"
#include "Log.h"

#include <boost/program_options.hpp>

#include <chrono>
#include <csignal>
#include <iomanip>
#include <sstream>
#include <thread>
#include <tuple>

namespace po = boost::program_options;

int main(int argc, char ** argv)
{
    std::string ticker;
    int64_t delay_ms;
    bool with_order_book;
    std::string domain;
    Port port;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("ticker", po::value<std::string>(&ticker)->default_value("BTCUSDT"), "set ticker, default [BTCUSDT]")
        ("period", po::value<int64_t>(&delay_ms)->default_value(5000), "set period between statistics output, default [5000]")
        ("with-orderbook", po::value<bool>(&with_order_book)->default_value(true), "prints order book from the best listener, default [true]")
        ("host", po::value<std::string>(&domain)->default_value("stream.binance.com"), "set host to connect, default [stream.binance.com]")
        ("port", po::value<Port>(&port)->default_value(9443), "set port to connect, default [9443]")

        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        ALWAYS_LOG(desc);
        return 1;
    }

    const auto dns_lookup = create_dns_resolver();

    const auto ips = dns_lookup->resolve(domain);

    LOG_LINE("Resolved IPs [" << ips.size() << "]:\n" << SequencePrinter(ips, "\n"));

    std::vector<std::pair<std::unique_ptr<IConnector>, DepthDataListenerPtr>> measurers;
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

    if (measurers.empty()) {
        ALWAYS_LOG("No IPs detected for " << domain);
        return -1;
    }

    static std::condition_variable cv;
    static std::mutex signal_mutex;
    static bool run = true;
    std::signal(SIGINT, [] ([[maybe_unused]] const int signal) {
        std::unique_lock lk(signal_mutex);
        run = false;
        cv.notify_one();
    });

    std::vector<std::tuple<std::string, Statistics, DepthDataListenerPtr>> stats;
    stats.reserve(measurers.size());
    while (run) {
        stats.clear();
        for (const auto & [connection, listener] : measurers) {
            stats.emplace_back(connection->get_host(), listener->get_statistics(), listener);
        }
        std::sort(stats.begin(), stats.end(), [] (const auto & a, const auto & b) { return !std::get<1>(a).empty() && std::get<1>(a).get_avg_time() < std::get<1>(b).get_avg_time(); });
        std::ostringstream oss;
        oss << "Statistics:\n";
        for (const auto & [host, s, listener_] : stats) {
            oss << std::setw(15) << host << ": " << s << "\n";
        }
        oss << "OrderBook from best listener:\n" << std::get<2>(stats[0])->get_order_book();
        ALWAYS_LOG(std::move(oss).str());
        std::unique_lock lk(signal_mutex); // synchronizes run variable
        cv.wait_for(lk, std::chrono::milliseconds(delay_ms));
    }
    return 0;
}