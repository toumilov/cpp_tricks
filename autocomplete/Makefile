SRC_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
NAME := ac_sample

CXXFLAGS += -std=c++17 -Wall -Werror -I$(SRC_DIR)

.PHONY: all autocomplete clean

all: autocomplete

autocomplete: autocomplete.cpp
	$(CXX) $< -o $(NAME)

clean:
	rm -f *.o
	rm -f $(NAME)
