
CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SONAME=libtest.so.1
FULL_LIB_NAME=libtest.so.1.0.0

.PHONY: all libtest app clean

all: libtest.so app

libtest.so: libtest.o
	$(CXX) $^ -shared -Wl,-soname,${SONAME} -fvisibility=hidden -o $(FULL_LIB_NAME)
	ln -sf $(FULL_LIB_NAME) $(SONAME)
	ln -sf $(FULL_LIB_NAME) $@

app: app.o
	$(CXX) $^ -L $(CURRENT_DIR) -ltest -ldl -Wl,-rpath,\$${ORIGIN} -o $@

libtest.o: $(CURRENT_DIR)libtest.cpp
	$(CXX) -I $(CURRENT_DIR) -Wall -fPIC -c -o $@ $<

app.o: $(CURRENT_DIR)app.cpp
	$(CXX) -I $(CURRENT_DIR) -c -o $@ $<

clean:
	rm -rf libtest.so*
	rm -rf app
	rm -rf *.o

