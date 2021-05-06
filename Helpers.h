#pragma once

#include <ostream>

template <class Cont>
class SequencePrinter
{
public:
    SequencePrinter(const Cont & cont, const char * separator = "\n")
        : m_cont(cont)
        , m_separator(separator)
    {}

    std::ostream & print(std::ostream & strm) const
    {
        for (auto it = m_cont.begin(); it != m_cont.end(); ++it) {
            strm << *it;
            if (it + 1 != m_cont.end()) {
                strm << m_separator;
            }
        }
        return strm;
    }

    friend std::ostream & operator<< (std::ostream & strm, const SequencePrinter & v) { return v.print(strm); }

private:
    const Cont & m_cont;
    const char * m_separator;
};