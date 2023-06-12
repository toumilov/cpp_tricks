
SRC_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(SRC_DIR)build

SOURCE = scope_exit.cpp

OBJ_FILES := $(SOURCE:%=$(BUILD_DIR)/%.o)
CXXFLAGS += -std=c++11 -Wall -Werror -I$(SRC_DIR)

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all test unique_ptr clean

all: test unique_ptr

test: $(OBJ_FILES)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

unique_ptr: scope_exit_unique_ptr.cpp
	$(CXX) $^ -o $@

clean:
	rm -fr $(BUILD_DIR)
	rm -f test
	rm -f unique_ptr

