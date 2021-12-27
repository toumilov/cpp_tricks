
CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CXX = g++
CPPFLAGS = -std=c++11

.PHONY: all app

all: app popen io

app: app.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

popen: popen.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

io: subprocess_io.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

clean:
	rm -rf $(CURRENT_DIR)*.o
	rm -rf $(CURRENT_DIR)popen
	rm -rf $(CURRENT_DIR)app
	rm -rf $(CURRENT_DIR)io
