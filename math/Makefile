
SRC_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CXXFLAGS += -std=c++17 -Wall -Werror -I$(SRC_DIR)

.PHONY: all ca sma

all: ca sma

ca: cumulative_average.o
	$(CXX) $< -o $@

sma: simple_moving_average.o
	$(CXX) $< -o $@

clean:
	rm -f *.o
	rm -f sma
	rm -f ca
