CC=g++
CXXFLAGS=-std=c++11

enumerable: enumerable.h main.cpp
	$(CC) $(CXXFLAGS) -o enumerable main.cpp 

clean:
	rm enumerable