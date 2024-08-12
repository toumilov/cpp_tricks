
CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CXX = g++

.PHONY: all server client clean

all: server client

ls.o: $(CURRENT_DIR)local_socket.cpp
	$(CXX) -I $(CURRENT_DIR) -c -o $@ $<

server: $(CURRENT_DIR)server.cpp ls.o
	$(CXX) -I $(CURRENT_DIR) -o $@ $^

client: $(CURRENT_DIR)client.cpp ls.o
	$(CXX) -I $(CURRENT_DIR) -o $@ $^

clean:
	rm -rf client
	rm -rf server
	rm *.o
