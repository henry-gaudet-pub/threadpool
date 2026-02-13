CXXSTD ?= c++17

all:
	g++ -std=$(CXXSTD) threadpool.cpp test.cpp -lpthread -o threadpool_test
clean:
	rm -f threadpool_test
