## BUILD
FogConnect depends on following packages:
- [openssl](https://www.cnblogs.com/emanlee/p/6100019.html)
- [libwebsockets.so.10](https://libwebsockets.org/)
- [libjansson.so.4](https://github.com/akheron/jansson)
- [libusrsctp.so.1](https://github.com/sctplab/usrsctp)
- [libevent-2.0.so.5](https://github.com/libevent/libevent)
- [libglib-2.0.so.0](https://github.com/GNOME/glib)


## Supported Environment
- Ubuntu/Linuxmint


### Ubuntu/LinuxMint

#### Prepare deps
Install common deps:

``` shell
sudo apt-get install git g++ make 
```

Install dependencies:

``` shell
sudo apt-get install openssl libssl-dev libwebsockets-dev libjansson-dev ibevent-dev libglib2.0-dev 
```

Install libusrsctp:
``` shell
git clone https://github.com/sctplab/usrsctp.git
cd usrsctp/
cmake .
make
sudo cp usrsctplib/libusrsctp.so* /usr/lib/x86_64-linux-gnu/
sudo cp usrsctplib/libusrsctp.a /usr/lib/x86_64-linux-gnu/
```
#### Compile FogConnect
##### Prepare the FogConnect env
``` shell
git clone git@github.com:PearInc/FogConnect.git
cd FogConnect
sudo cp include/fogconnect.h /usr/include/
sudo cp x86/linux/64/libfog* /usr/lib/x86_64-linux-gnu/
```

##### Build and run examples
``` shell
make
./x86/linux/64/test_server && ./x86/linux/64/test_client
```
This example default use the QUIC protocol