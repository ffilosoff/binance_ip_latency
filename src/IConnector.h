#pragma once

#include "IPAddress.h"

class IConnector {
public:
    virtual ~IConnector() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual bool is_running() const = 0;

    virtual IPAddress get_host() const = 0;
};