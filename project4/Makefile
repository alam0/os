CXX=		g++
CXXFLAGS=	-g -Wall -std=gnu++11
SHELL=		bash

all:		main   processStrings  getInMemory

processStrings:   processStrings.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

getInMemory:   getInMemory.c
	$(CXX) $(CXXFLAGS) -o $@ $^ -lcurl

main.o:	main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp
	
Process.o:	Process.cpp
	$(CXX) $(CXXFLAGS) -c Process.cpp

main:   main.o Process.o 
	$(CXX) $(CXXFLAGS) main.o Process.o -o main
	

clean:
	rm -f processStrings getInMemory main *.o *~


