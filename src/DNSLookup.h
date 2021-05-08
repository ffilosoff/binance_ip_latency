#pragma once

#include "IPAddress.h"

#include <memory>
#include <vector>

class IDNSLookup {
public:
    virtual ~IDNSLookup() = default;
    virtual std::vector<IPAddress> resolve(const std::string & domain) = 0;
};

std::unique_ptr<IDNSLookup> create_dns_resolver();