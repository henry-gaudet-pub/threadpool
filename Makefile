all: 
	g++ threadpool.cpp test.cpp -lpthread -o threadpool_test
clean:
	rm threadpool_test
