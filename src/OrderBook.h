#pragma once

#include <iosfwd>
#include <vector>

class OrderBook // TODO: write tests
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
    void clear();

    const auto & get_bids() const { return m_bids; }
    const auto & get_asks() const { return m_asks; }

    bool empty() const { return m_bids.empty() && m_asks.empty(); }

    std::ostream & print(std::ostream &, size_t levels_num = -1) const;

    friend std::ostream & operator<< (std::ostream & strm, const OrderBook & ob) { return ob.print(strm); }

private:
    template <class Comp>
    static void insert_replace(std::vector<Level> & lvls, const Price &, const Volume &, Comp);

public:
    std::vector<Level> m_bids;
    std::vector<Level> m_asks;
};
