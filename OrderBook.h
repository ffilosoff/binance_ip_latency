#pragma once

#include <vector>

class OrderBook
{
    using Price = double;
    using Volume = double;
    struct Level
    {
        Level(Price p, Volume v)
            : price(std::move(p))
            , volume(std::move(v))
        {}

        Price price;
        Volume volume;
    };
    struct BidComparator;
    struct AskComparator;

public:
    enum class Side
    {
        Bid,
        Ask,
    };

    void insert_replace(const Price &, const Volume &, Side);

private:
    template <class Comp>
    static void insert_replace(std::vector<Level> & lvls, const Price &, const Volume &, Comp);

public:
    std::vector<Level> m_bids;
    std::vector<Level> m_asks;
};