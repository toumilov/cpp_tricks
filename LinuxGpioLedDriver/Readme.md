
## This is a simple GPIO LED driver for Raspberry Pi.
### _Developed and tested on Raspberry Pi 4 model B on Raspbian._

### Package contents:

* ledgpio.c          - Linux kernel driver (LKM).
* ledgpio.h          - Driver definitions used by driver and user space applications.
* ledgpio_client.h   - C++ client class header.
* ledgpio_client.cpp - C++ client class implementation.
* test.cpp           - User space client application.
* Makefile           - Build targets definition.

### Build instructions:
In source folder run "make".

Linux kernel headers and C/C++ compiler required.

_Tested with g++._

### Features:
Driver provides access to GPIO ports 2 - 26.
Interface consist of four commands:
* LED_OP_START - initialize a set of ports that are needed for further operations;
* LED_OP_STOP - release ports and turn them off;
* LED_OP_SET - set ports state (uninitialized ports are not affected);
* LED_OP_GET - get current port state stored in driver;

_Note: only one client can access driver at a time. concurrent access is not supported._

### Test driver
1. Load driver: **_sudo insmod ledgpio.ko_**
2. Check _/var/log/kern.log_ if module loaded successfully
3. Run test app: **_sudo ./test_**
4. Unload driver: **_sudo rmmod ledgpio.ko_**
