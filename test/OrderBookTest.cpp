#include "../src/OrderBook.h"

#include <gtest/gtest.h>

#define ENABLE_DEBUG_PRINT

#ifdef ENABLE_DEBUG_PRINT
#define DEBUG_PRINT(msg) std::cout << msg << std::endl;
#else
#define DEBUG_PRINT(msg) (void)
#endif

TEST(OrderBookTest, empty_order_book) {
    OrderBook ob;
    DEBUG_PRINT(ob);
    ASSERT_TRUE(ob.empty());
}

TEST(OrderBookTest, insert_one_bid_lvl) {
    OrderBook ob;
    ob.insert_replace(0.1, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ASSERT_FALSE(ob.empty()) << ob;
    const auto & bids = ob.get_bids();
    ASSERT_FALSE(bids.empty());
    const auto & asks = ob.get_asks();
    ASSERT_TRUE(asks.empty());
}

TEST(OrderBookTest, insert_one_ask_lvl) {
    OrderBook ob;
    ob.insert_replace(0.1, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ASSERT_FALSE(ob.empty()) << ob;
    const auto & bids = ob.get_bids();
    ASSERT_TRUE(bids.empty());
    const auto & asks = ob.get_asks();
    ASSERT_FALSE(asks.empty());
}

TEST(OrderBookTest, insert_bid_lvl_before_existing) {
    OrderBook ob;
    ob.insert_replace(0.1, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.2, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    const auto & bids = ob.get_bids();
    ASSERT_EQ(bids.size(), 2);
    ASSERT_DOUBLE_EQ(bids[0].price, 0.2);
    ASSERT_DOUBLE_EQ(bids[1].price, 0.1);
}

TEST(OrderBookTest, insert_ask_lvl_before_existing) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    const auto & asks = ob.get_asks();
    ASSERT_EQ(asks.size(), 2);
    ASSERT_DOUBLE_EQ(asks[0].price, 0.1);
    ASSERT_DOUBLE_EQ(asks[1].price, 0.2);
}

TEST(OrderBookTest, insert_bid_lvl_after_existing) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    const auto & bids = ob.get_bids();
    ASSERT_EQ(bids.size(), 2);
    ASSERT_DOUBLE_EQ(bids[0].price, 0.2);
    ASSERT_DOUBLE_EQ(bids[1].price, 0.1);
}

TEST(OrderBookTest, insert_ask_lvl_after_existing) {
    OrderBook ob;
    ob.insert_replace(0.1, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.2, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    const auto & asks = ob.get_asks();
    ASSERT_EQ(asks.size(), 2);
    ASSERT_DOUBLE_EQ(asks[0].price, 0.1);
    ASSERT_DOUBLE_EQ(asks[1].price, 0.2);
}

TEST(OrderBookTest, insert_bid_lvl_in_between) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    const auto & bids = ob.get_bids();
    ASSERT_EQ(bids.size(), 3);
    ASSERT_DOUBLE_EQ(bids[0].price, 0.2);
    ASSERT_DOUBLE_EQ(bids[1].price, 0.15);
    ASSERT_DOUBLE_EQ(bids[2].price, 0.1);
}

TEST(OrderBookTest, insert_ask_lvl_in_between) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    const auto & asks = ob.get_asks();
    ASSERT_EQ(asks.size(), 3);
    ASSERT_DOUBLE_EQ(asks[0].price, 0.1);
    ASSERT_DOUBLE_EQ(asks[1].price, 0.15);
    ASSERT_DOUBLE_EQ(asks[2].price, 0.2);
}

TEST(OrderBookTest, replace_bid_lvl_in_between) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 0.25, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    const auto & bids = ob.get_bids();
    ASSERT_EQ(bids.size(), 3);
    ASSERT_DOUBLE_EQ(bids[0].price, 0.2);
    ASSERT_DOUBLE_EQ(bids[1].price, 0.15);
    ASSERT_DOUBLE_EQ(bids[2].price, 0.1);
    ASSERT_DOUBLE_EQ(bids[1].volume, 0.25);
}

TEST(OrderBookTest, replace_ask_lvl_in_between) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 25, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    const auto & asks = ob.get_asks();
    ASSERT_EQ(asks.size(), 3);
    ASSERT_DOUBLE_EQ(asks[0].price, 0.1);
    ASSERT_DOUBLE_EQ(asks[1].price, 0.15);
    ASSERT_DOUBLE_EQ(asks[2].price, 0.2);
    ASSERT_DOUBLE_EQ(asks[1].volume, 25);
}

TEST(OrderBookTest, replace_first_bid_lvl) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.2, 0.25, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    const auto & bids = ob.get_bids();
    ASSERT_EQ(bids.size(), 3);
    ASSERT_DOUBLE_EQ(bids[0].price, 0.2);
    ASSERT_DOUBLE_EQ(bids[1].price, 0.15);
    ASSERT_DOUBLE_EQ(bids[2].price, 0.1);
    ASSERT_DOUBLE_EQ(bids[0].volume, 0.25);
}

TEST(OrderBookTest, replace_first_ask_lvl) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 25, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    const auto & asks = ob.get_asks();
    ASSERT_EQ(asks.size(), 3);
    ASSERT_DOUBLE_EQ(asks[0].price, 0.1);
    ASSERT_DOUBLE_EQ(asks[1].price, 0.15);
    ASSERT_DOUBLE_EQ(asks[2].price, 0.2);
    ASSERT_DOUBLE_EQ(asks[0].volume, 25);
}

TEST(OrderBookTest, replace_last_bid_lvl) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 0.25, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    const auto & bids = ob.get_bids();
    ASSERT_EQ(bids.size(), 3);
    ASSERT_DOUBLE_EQ(bids[0].price, 0.2);
    ASSERT_DOUBLE_EQ(bids[1].price, 0.15);
    ASSERT_DOUBLE_EQ(bids[2].price, 0.1);
    ASSERT_DOUBLE_EQ(bids[2].volume, 0.25);
}

TEST(OrderBookTest, replace_last_ask_lvl) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.2, 25, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    const auto & asks = ob.get_asks();
    ASSERT_EQ(asks.size(), 3);
    ASSERT_DOUBLE_EQ(asks[0].price, 0.1);
    ASSERT_DOUBLE_EQ(asks[1].price, 0.15);
    ASSERT_DOUBLE_EQ(asks[2].price, 0.2);
    ASSERT_DOUBLE_EQ(asks[2].volume, 25);
}

TEST(OrderBookTest, remove_bid_lvl_in_between) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 0.0, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    const auto & bids = ob.get_bids();
    ASSERT_EQ(bids.size(), 2);
    ASSERT_DOUBLE_EQ(bids[0].price, 0.2);
    ASSERT_DOUBLE_EQ(bids[1].price, 0.1);
}

TEST(OrderBookTest, remove_ask_lvl_in_between) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 0.0, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    const auto & asks = ob.get_asks();
    ASSERT_EQ(asks.size(), 2);
    ASSERT_DOUBLE_EQ(asks[0].price, 0.1);
    ASSERT_DOUBLE_EQ(asks[1].price, 0.2);
}

TEST(OrderBookTest, remove_first_bid_lvl) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.2, 0.0, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    const auto & bids = ob.get_bids();
    ASSERT_EQ(bids.size(), 2);
    ASSERT_DOUBLE_EQ(bids[0].price, 0.15);
    ASSERT_DOUBLE_EQ(bids[1].price, 0.1);
}

TEST(OrderBookTest, remove_first_ask_lvl) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 0.0, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    const auto & asks = ob.get_asks();
    ASSERT_EQ(asks.size(), 2);
    ASSERT_DOUBLE_EQ(asks[0].price, 0.15);
    ASSERT_DOUBLE_EQ(asks[1].price, 0.2);
}

TEST(OrderBookTest, remove_last_bid_lvl) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 0.0, OrderBook::Side::Bid);
    DEBUG_PRINT(ob);
    const auto & bids = ob.get_bids();
    ASSERT_EQ(bids.size(), 2);
    ASSERT_DOUBLE_EQ(bids[0].price, 0.2);
    ASSERT_DOUBLE_EQ(bids[1].price, 0.15);
}

TEST(OrderBookTest, remove_last_ask_lvl) {
    OrderBook ob;
    ob.insert_replace(0.2, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.1, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.15, 1, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    ob.insert_replace(0.2, 0.0, OrderBook::Side::Ask);
    DEBUG_PRINT(ob);
    const auto & asks = ob.get_asks();
    ASSERT_EQ(asks.size(), 2);
    ASSERT_DOUBLE_EQ(asks[0].price, 0.1);
    ASSERT_DOUBLE_EQ(asks[1].price, 0.15);
}