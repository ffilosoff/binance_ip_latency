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
    std::string ticker = "BTCUSDT";
    int64_t delay_ms = 5000;
    bool with_order_book = true;
    size_t max_ob_levels_to_show = -1;
    std::string domain = "stream.binance.com";
    Port port = 9443;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("ticker", po::value<std::string>(&ticker)->default_value("BTCUSDT"), "set ticker")
        ("period", po::value<int64_t>(&delay_ms)->default_value(5000), "set period between statistics output")
        ("with-orderbook", po::value<bool>(&with_order_book)->default_value(true), "prints order book from the best listener")
        ("show-orderbook-levels-num", po::value<size_t>(&max_ob_levels_to_show)->default_value(-1), "set number of levels for orderbook to output, default -1, i.e. all")
        ("host", po::value<std::string>(&domain)->default_value("stream.binance.com"), "set host to connect")
        ("port", po::value<Port>(&port)->default_value(9443), "set port to connect")

        ;

    po::variables_map vm;

    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm); // call after last store, and before accesing parsed variables
    if (vm.count("help")) {
        ALWAYS_LOG(desc);
        return 1;
    }

    const auto period = std::chrono::milliseconds(delay_ms);
    ALWAYS_LOG(
        "Configuration:"
        << "\n Ticker: " << ticker
        << "\n Period: " << period.count() << "ms"
        << "\n Build order book: " << std::boolalpha << with_order_book
        << "\n Max OB levels num to show: " << max_ob_levels_to_show
        << "\n Host: " << domain
        << "\n Port: " << port);

    const auto dns_lookup = create_dns_resolver();

    const auto ips = dns_lookup->resolve(domain);

    LOG_LINE("Resolved IPs [" << ips.size() << "]:\n" << SequencePrinter(ips, "\n"));

    std::vector<std::pair<std::unique_ptr<IConnector>, DepthDataListenerPtr>> measurers;
    measurers.reserve(ips.size());
    for (const auto & ip : ips) {
        auto listener = std::make_shared<binance::BinanceIncDepthProcessor>(with_order_book);
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
        bool any_running = false;
        for (const auto & [connection, listener] : measurers) {
            if (connection->is_running()) {
                any_running = true;
                stats.emplace_back(connection->get_host(), listener->get_statistics(), listener);
            }
        }
        if (!any_running) {
            ALWAYS_LOG("All connections are down, time to stop");
            break;
        }
        std::sort(stats.begin(), stats.end(), [] (const auto & a, const auto & b) {
            if (std::get<1>(a).empty()) {
                return false;
            }
            return std::get<1>(a).get_avg_time() < std::get<1>(b).get_avg_time();
        });
        std::ostringstream oss;
        oss << "Statistics:\n";
        for (const auto & [host, s, listener_] : stats) {
            oss << std::setw(15) << host << ": " << s << "\n";
        }
	if (with_order_book) {
            oss << "OrderBook from the best listener:\n";
	    auto & listener = std::get<2>(stats[0]);
	    if (listener) {
	        std::get<2>(stats[0])->get_order_book().print(oss, max_ob_levels_to_show);
	    } else {
                ALWAYS_LOG("[ERROR]: unexpected empty listener when with_order_book=" << std::boolalpha << with_order_book);
	    }
        }
        ALWAYS_LOG(std::move(oss).str());
        std::unique_lock lk(signal_mutex); // synchronizes run variable
        cv.wait_for(lk, std::chrono::milliseconds(delay_ms));
    }
    ALWAYS_LOG("Stopping measurers");
    for (auto & m : measurers) {
        m.first->stop();
    }
    return 0;
}
