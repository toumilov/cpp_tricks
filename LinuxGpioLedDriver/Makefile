obj-m+=ledgpio.o

all: test.o ledgpio_client.o
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CXX) test.o ledgpio_client.o -o test

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm test

ledgpio_client.o: ledgpio_client.cpp
	echo "<ledgpio_client.o>"
	$(CXX) -c ledgpio_client.cpp -o ledgpio_client.o

test.o: test.cpp
	echo "<test.o>"
	$(CXX) -c test.cpp -o test.o
