
CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CXX = g++
#CPPFLAGS = -std=c++11
LIBS = -pthread

.PHONY: all semaphore

all: semaphore flock

semaphore: semaphore.cpp
	$(CXX) $(CPPFLAGS) $(LIBS) $^ -o $@

flock: file_lock.cpp
	$(CXX) $(CPPFLAGS) $(LIBS) $^ -o $@

clean:
	rm -rf $(CURRENT_DIR)*.o
	rm -rf $(CURRENT_DIR)semaphore
	rm -rf $(CURRENT_DIR)flock
