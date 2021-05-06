#include <iostream>

#include "DNSLookup.h"
#include "Helpers.h"
#include "Log.h"

#include <sstream>

int main() {
    const auto dns_lookup = create_dns_resolver();

    constexpr auto domain = "stream.binance.com";

    const auto ips = dns_lookup->resolve(domain);

    LOG_LINE("Resolved IPs [" << ips.size() << "]:\n" << SequencePrinter(ips, "\n"));



    return 0;
}