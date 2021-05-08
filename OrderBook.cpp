#include "OrderBook.h"

#include <algorithm>
#include <cassert>

struct Double
{
    inline static constexpr double EPS = 10e-7;

    static bool less(const double v1, const double v2, const double eps = EPS)
    {
        return v1 + EPS < v2;
    }

    static bool greater(const double v1, const double v2, const double eps = EPS)
    {
        return v1 > v2 + EPS;
    }

    static bool equal(const double v1, const double v2, const double eps = EPS)
    {
        return std::abs(v1 - v2) <= eps;
    }
};

struct OrderBook::BidComparator
{
    bool operator() (const Level & lvl, const Price & p)
    {
        return Double::less(lvl.price, p);
    }

    bool operator() (const Price & p, const Level & lvl)
    {
        return Double::less(p, lvl.price);
    }
};

struct OrderBook::AskComparator
{
    bool operator() (const Level & lvl, const Price & p)
    {
        return Double::greater(lvl.price, p);
    }

    bool operator() (const Price & p, const Level & lvl)
    {
        return Double::greater(p, lvl.price);
    }
};

template <class Comp>
void OrderBook::insert_replace(std::vector<Level> & lvls, const Price & p, const Volume & v, const Comp comp)
{
    auto it = std::lower_bound(lvls.begin(), lvls.end(), p, comp);
    if (it != lvls.begin()) {
        --it;
    }
    if (Double::equal(it->price, p)) { // replace lvl
        it->price = p;
        it->volume = v;
    } else { // insert new lvl
        lvls.insert(it, Level(p, v));
    }
}

void OrderBook::insert_replace(const Price & p, const Volume & v, const Side s)
{
    switch (s) {
    case Side::Bid:
        insert_replace(m_bids, p, v, BidComparator());
        break;
    case Side::Ask:
        insert_replace(m_bids, p, v, AskComparator());
        break;
    default:
        assert(false);
    }
}
