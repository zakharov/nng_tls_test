A test scenario for the TLS nng

Build:
````
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
````

Enable server sanitizer:
````
cmake .. -DMCX_SANITIZER=address
````

Usage:
````
nng_tls_server [URL]
nng_tls_client [URL]
Default URL: wss://localhost:5555
````
