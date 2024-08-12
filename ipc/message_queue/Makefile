SRC_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(SRC_DIR)build

SOURCE = mq_test.cpp

OBJ_FILES := $(SOURCE:%=$(BUILD_DIR)/%.o)
CXXFLAGS += -std=c++17 -g -Wall -Werror -I$(SRC_DIR)
LDLIBS := -lpthread
LDFLAGS :=

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all test clean

all: test

test: $(OBJ_FILES)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -fr $(BUILD_DIR)
	rm -f test
