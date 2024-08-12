
SRC_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(SRC_DIR)build

SOURCE = logger.cpp \
		 socket.cpp \
		 thread_pool.cpp \
		 session.cpp \
		 proxy.cpp \
		 main.cpp

OBJ_FILES := $(SOURCE:%=$(BUILD_DIR)/%.o)
CXXFLAGS += -std=c++17 -Wall -Werror -I$(SRC_DIR)
LDLIBS := -lpthread
LDFLAGS :=

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all proxy clean

all: proxy

proxy: $(OBJ_FILES)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -fr $(BUILD_DIR)
	rm -f proxy

