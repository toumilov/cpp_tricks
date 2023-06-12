
SRC_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CXXFLAGS += -std=c++17 -Wall -Werror -I$(SRC_DIR)

.PHONY: all rand

all: rand

rand: random.cpp
	$(CXX) $< -o $@

clean:
	rm -f *.o
	rm -f rand
