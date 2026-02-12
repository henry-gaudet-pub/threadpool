all:
	g++ -std=c++11 threadpool.cpp test.cpp -lpthread -o threadpool_test
clean:
	rm -f threadpool_test
