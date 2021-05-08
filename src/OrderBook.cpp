#include "OrderBook.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>

struct Double
{
    inline static constexpr double EPS = 10e-7;

    static int compare(const double v1, const double v2, const double eps = EPS)
    {
        const auto diff = v1 - v2;
        if (diff >= -eps && eps >= diff) {
            return 0;
        }
        return diff < 0 ? -1 : 1;
    }

    static bool less(const double v1, const double v2, const double eps = EPS)
    {
        return v1 + eps < v2;
    }

    static bool greater(const double v1, const double v2, const double eps = EPS)
    {
        return v1 > v2 + eps;
    }

    static bool equal(const double v1, const double v2, const double eps = EPS)
    {
        return std::abs(v1 - v2) <= eps;
    }
};

struct OrderBook::AskComparator
{
    int compare(const Price & p1, const Price & p2) const
    {
        return Double::compare(p1, p2);
    }

    bool operator() (const Level & lvl, const Price & p) const
    {
        return Double::less(lvl.price, p);
    }

    bool operator() (const Price & p, const Level & lvl) const
    {
        return Double::less(p, lvl.price);
    }
};

struct OrderBook::BidComparator
{
    int compare(const Price & p1, const Price & p2) const
    {
        return -Double::compare(p1, p2);
    }

    bool operator() (const Level & lvl, const Price & p) const
    {
        return Double::greater(lvl.price, p);
    }

    bool operator() (const Price & p, const Level & lvl) const
    {
        return Double::greater(p, lvl.price);
    }
};

template <class Comp>
void OrderBook::insert_replace(std::vector<Level> & lvls, const Price & p, const Volume & v, const Comp comp)
{
    const bool remove_lvl = Double::equal(v, 0);
    auto it = std::lower_bound(lvls.begin(), lvls.end(), p, comp);
    if (it == lvls.end()) {
        if(!remove_lvl) { // FIXME: probably volume is too low to fit even event from binance
            lvls.emplace_back(p, v);
        }
        return;
    }
    const auto res = comp.compare(p, it->price);
    switch (res) {
    case 0:
        if (remove_lvl) {
            lvls.erase(it);
        } else {
            it->price = p;
            it->volume = v;
        }
        break;
    case -1:
        if (!remove_lvl) {
            lvls.insert(it, Level(p, v));
        }
        break;
    case 1:
        if (!remove_lvl) {
            lvls.insert(it == lvls.begin() ? it : it - 1, Level(p, v));
        }
        break;
    }
}

void OrderBook::insert_replace(const Price & p, const Volume & v, const Side s)
{
    switch (s) {
    case Side::Bid:
        insert_replace(m_bids, p, v, BidComparator());
        break;
    case Side::Ask:
        insert_replace(m_asks, p, v, AskComparator());
        break;
    default:
        assert(false);
    }
}

void OrderBook::clear()
{
    m_bids.clear();
    m_asks.clear();
}

std::ostream & OrderBook::print(std::ostream & strm, const size_t levels_to_show) const
{
    const auto print_pre = [] (auto & strm) { strm << "\n|"; };
    const auto print_lvl = [] (auto & strm, const auto & lvls, const auto idx) {
        if (idx < lvls.size()) {
            strm << "| " << std::setw(10) << std::right << lvls[idx].volume << "@" << std::setw(16) << std::left << std::setprecision(7) << lvls[idx].price << " |";
        } else {
            strm << "|            <empty>          |";
        }
    };
    const auto print_post = [] (auto & strm) { strm << "|"; };
    const auto max_sz = std::max(m_asks.size(), m_bids.size());
    strm << "OrderBook: BIDS: " << m_bids.size() << ", ASKS: " << m_asks.size();
    if (levels_to_show == -1) {
        strm << ", showing ALL levels";
    } else {
        strm << ", showing max " << levels_to_show << " levels";
    }
    strm << "\n||            BIDS             ||" << "            ASKS             ||";
    for (size_t idx = 0; idx < std::min(max_sz, levels_to_show); ++idx) {
        print_pre(strm);
        print_lvl(strm, m_bids, idx);
        print_lvl(strm, m_asks, idx);
        print_post(strm);
    }
    if (max_sz == 0) { // print empty
        print_pre(strm);
        print_lvl(strm, m_bids, 0);
        print_lvl(strm, m_asks, 0);
        print_post(strm);
    }
    return strm;
}
