Pre-requirements (tested):

1. Ubuntu 20.04.2 LTS
2. Boost 1.71 (program_options, system, asio, beast)
3. OpenSSL 1.1.1f
4. RapidJSON 1.1.0
5. (optionally) Google Test 1.10.0-2

Build:
```
$ mkdir build
$ mkdir build && cd build && cmake -c ../CMakeLists.txt -B . && make
```

Test:
```
$ make test
```

Usage:
```
Allowed options:
  --help                                produce help message
  --ticker arg (=BTCUSDT)               set ticker
  --period arg (=5000)                  set period between statistics output
  --with-orderbook arg (=1)             prints order book from the best 
                                        listener
  --show-orderbook-levels-num arg (=18446744073709551615)
                                        set number of levels for orderbook to 
                                        output, default -1, i.e. all
  --host arg (=stream.binance.com)      set host to connect
  --port arg (=9443)                    set port to connect
```
