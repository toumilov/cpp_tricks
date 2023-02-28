
CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CXX = g++

.PHONY: all filter clean

all: filter

filter: $(CURRENT_DIR)filter.cpp
	$(CXX) -I $(CURRENT_DIR) -o $@ $^

clean:
	rm -rf filter
	rm *.o
