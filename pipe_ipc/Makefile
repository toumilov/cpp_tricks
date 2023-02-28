
CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CXX = g++

.PHONY: all server client clean

all: server client

server: $(CURRENT_DIR)server.cpp
	$(CXX) -I $(CURRENT_DIR) -o $@ $^

client: $(CURRENT_DIR)client.cpp
	$(CXX) -I $(CURRENT_DIR) -o $@ $^

clean:
	rm -rf client
	rm -rf server
	rm *.o
