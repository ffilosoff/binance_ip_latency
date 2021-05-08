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
