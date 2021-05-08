Pre-requirements (tested):

Ubuntu 20.04.2 LTS
Boost 1.71 (program_options, system, asio, beast)
OpenSSL 1.1.1f
RapidJSON 1.1.0
(optionally) Google Test 1.10.0-2

Build:
$ mkdir build
$ mkdir build && cd build && cmake -c ../CMakeLists.txt -B . && make

Test:
$ make test
