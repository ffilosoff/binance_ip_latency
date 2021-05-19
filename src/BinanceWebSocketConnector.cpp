#include "BinanceWebSocketConnector.h"

#include "Log.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <thread>
#include <utility>

namespace binance {

namespace asio = boost::asio;
namespace beast = boost::beast;

#define _LOG(msg) LOG_LINE("[" << m_endpoint.address() << ":" << m_endpoint.port() << "]: " << msg)
#define _LOG_ALWAYS(msg) ALWAYS_LOG("[" << m_endpoint.address() << ":" << m_endpoint.port() << "]: " << msg)
#define report_error(ec) \
    do { \
        if (m_running.load(std::memory_order_acquire)) \
        { \
            _LOG("ERROR: " << ec.value() << ", " << ec.message()); \
            if (m_data_listener) { \
                m_data_listener->failure(ec.message()); \
            } \
            m_failure_reason = ec.message(); \
            m_running.store(false, std::memory_order_release); \
        } \
    } while (0)

#define report_str_error(err) \
    do { \
        if (m_running.load(std::memory_order_acquire)) \
        { \
            _LOG("ERROR: " << err); \
            if (m_data_listener) { \
                m_data_listener->failure(err); \
            } \
            m_failure_reason = err; \
            m_running.store(false, std::memory_order_release); \
        } \
    } while(0)

namespace {
std::string to_lower(std::string str)
{
    for (auto & v : str) {
        v = std::tolower(static_cast<char>(v));
    }
    return str;
}
}

class BinanceWebSocketConnector::Impl
{
public:
    Impl(const IPAddress & ip, const Port & port, std::string request, JsonDataListenerPtr listener)
        : m_request(std::move(request))
        , m_endpoint(asio::ip::make_address_v4(ip), port)
        , m_ssl_context(boost::asio::ssl::context::sslv23_client)
        , m_ws(m_io_context, m_ssl_context)
        , m_data_listener(std::move(listener))
    {
        _LOG("ctor: " << m_request);
    }

    ~Impl()
    {
        _LOG("dtor: " << m_request);
        stop();
    }

    void start()
    {
        auto ep = asio::ip::tcp::resolver::results_type::create(m_endpoint, m_endpoint.address().to_string(), std::to_string(m_endpoint.port()));
        asio::async_connect(m_ws.next_layer().next_layer(), ep, [this] (const auto ec, const auto & it) {
            _LOG("connection successful");
            if (ec) {
                report_error(ec);
            } else {
                setup_keep_alive();
                ssl_handshake();
            }
        });

        m_running.store(true, std::memory_order_release);
        m_thread = std::thread([this] {
            _LOG("executing thread");
            do {
                boost::system::error_code ec;
                m_io_context.run_for(std::chrono::seconds(1));
                setup_ping();
            } while (m_running.load(std::memory_order_acquire));
            if (!m_failure_reason.empty()) {
                _LOG_ALWAYS("stopped because of [" << m_failure_reason << "]");
            }
        });
    }

    void stop()
    {
        m_running.store(false, std::memory_order_release);
        if (!m_io_context.stopped()) {
            m_io_context.stop();
        }
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    auto get_host() const
    {
        return m_endpoint.address().to_string();
    }

    bool is_running() const
    {
        return m_running.load(std::memory_order_acquire);
    }

private:
    void setup_keep_alive()
    {
        _LOG("setup_keep_alive()");
        m_ws.control_callback(
            [this] ([[maybe_unused]] const beast::websocket::frame_type kind, [[maybe_unused]] const beast::string_view payload) mutable {
                _LOG("kind: " << (int)kind << ", payload: " << payload);
                switch (kind) {
                    case beast::websocket::frame_type::ping:
                    {
                        m_ws.async_pong(beast::websocket::ping_data{}, [this](const auto ec) {
                            _LOG("async_pong(): " << ec);
                            if (ec) {
                                report_error(ec);
                            }
                        });
                        break;
                    }
                    case beast::websocket::frame_type::pong:
                    {
                        const auto prev = m_ping_sent.exchange(false);
                        _LOG("Received pong, previouslt ping was sent: " << std::boolalpha << prev);
                        break;
                    }
                    default:
                        break;
                }
            }
        );
    }

    void setup_ping()
    {
        m_ws.async_ping(beast::websocket::ping_data{}, [this] (const auto ec) {
            _LOG("async_ping(): " << ec);
            if (m_ready.load(std::memory_order_acquire)) {
                if (ec) {
                    report_error(ec);
                } else {
                    const auto prev = m_ping_sent.exchange(true);
                    if (prev) {
                        report_str_error("Previous ping was not ponged");
                    }
                }
            }
        });
    }

    void ssl_handshake()
    {
        _LOG("ssl_handshake()");
        m_ws.next_layer().async_handshake(asio::ssl::stream_base::client,
            [this] (const auto ec) mutable {
                _LOG("ssl_handshake successful");
                if (ec) {
                    report_error(ec);
                } else {
                    setup_reader();
                }
            }
        );
    }

    void setup_reader()
    {
        _LOG("setup_reader()");
        m_ws.async_handshake(
            m_endpoint.address().to_string(), m_request, [this](const auto ec) mutable {
                _LOG("start reading");
                if (ec) {
                    report_error(ec);
                } else {
                    m_ready.store(true, std::memory_order_release);
                    setup_next_read();
                }
            }
        );
    }

    void setup_next_read()
    {
        _LOG("setup_read()");
        m_ws.async_read(
            m_buffer,
            [this] (const auto ec, const std::size_t size) mutable
            {
                _LOG("reading buffer");
                if (ec) {
                    report_error(ec);
                } else {
                    read(size);
                }
            }
        );
    }

    void read(const std::size_t sz)
    {
        const auto size = m_buffer.size();

        if (size != sz) {
            report_str_error("Internal error read buffer != provided size");
            return;
        }
        m_json_buffer.clear();
        for (const auto & b : m_buffer.data()) {
            m_json_buffer.append((const char *) b.data(), b.size());
        }
        _LOG("read json buffer: " << m_json_buffer);

        if (m_data_listener) {
            if (!m_data_listener->process(m_json_buffer)) {
                report_str_error("could not update depth");
                return;
            }
        }
        m_buffer.consume(size);

        setup_next_read();
    }

private:
    std::atomic<bool> m_running = false;
    std::thread m_thread;
    std::atomic<bool> m_ready {false};
    std::atomic<bool> m_ping_sent {false};
    std::string m_failure_reason;

    std::string m_request;
    asio::ip::tcp::endpoint m_endpoint;

    asio::io_context m_io_context;
    asio::ssl::context m_ssl_context;
    beast::websocket::stream<asio::ssl::stream<asio::ip::tcp::socket>> m_ws;
    boost::beast::multi_buffer m_buffer;
    std::string m_json_buffer;

    JsonDataListenerPtr m_data_listener;
};

BinanceWebSocketConnector::BinanceWebSocketConnector(const IPAddress & ip, const Port & port, std::string request, JsonDataListenerPtr listener)
    : m_impl(std::make_unique<BinanceWebSocketConnector::Impl>(ip, port, std::move(request), std::move(listener)))
{ }

BinanceWebSocketConnector::~BinanceWebSocketConnector() = default;

void BinanceWebSocketConnector::start()
{
    return m_impl->start();
}

void BinanceWebSocketConnector::stop()
{
    return m_impl->stop();
}

bool BinanceWebSocketConnector::is_running() const
{
    return m_impl->is_running();
}

IPAddress BinanceWebSocketConnector::get_host() const
{
    return m_impl->get_host();
}

std::unique_ptr<BinanceWebSocketConnector> BinanceWebSocketConnector::make_depth_connector(
    const IPAddress & ip,
    const Port & port,
    std::string ticker,
    JsonDataListenerPtr listener)
{
    constexpr auto updatetime = ""; //"@100ms"; // Seems that without @100ms flow is faster (need to check)
    return std::make_unique<BinanceWebSocketConnector>(ip, port, "/ws/" + to_lower(std::move(ticker)) + "@depth" + updatetime, std::move(listener));
}

}