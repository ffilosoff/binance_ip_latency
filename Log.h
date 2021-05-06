#pragma once

#define ENABLE_LOGGING

#ifdef ENABLE_LOGGING
    #include <iostream>
    #define LOG_LINE(msg) do { std::cout << __FILE__ << "(" << __LINE__ << "): "<< msg << "\n"; } while (0)
#else
    #define LOG_LINE(msg)
#endif