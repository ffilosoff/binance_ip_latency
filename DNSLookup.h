#pragma once

#include <memory>
#include <vector>
#include <string>

using IPAddressV4 = std::string;

class IDNSLookup {
public:
    virtual ~IDNSLookup() = default;
    virtual std::vector<IPAddressV4> resolve(const std::string & domain) = 0;
};

std::unique_ptr<IDNSLookup> create_dns_resolver();