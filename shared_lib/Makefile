
CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

.PHONY: all libtest app clean

all: libtest.so app

libtest.so: libtest.o
	$(CXX) $^ -shared -fvisibility=hidden -o $@

app: app.o
	$(CXX) $^ -L $(CURRENT_DIR) -ltest -ldl -Wl,-rpath=$(CURRENT_DIR) -o $@

libtest.o: $(CURRENT_DIR)libtest.cpp
	$(CXX) -I $(CURRENT_DIR) -Wall -fPIC -c -o $@ $<

app.o: $(CURRENT_DIR)app.cpp
	$(CXX) -I $(CURRENT_DIR) -c -o $@ $<

clean:
	rm -rf libtest.so
	rm -rf app
	rm -rf *.o

