CXX=g++
RM=rm -rf
CXXFLAGS=-std=c++11 -g -Wall
SRC=fakesrv.cpp
HDR=fakesrv.hpp
EXECUTABLE=fakesrv

.PHONY: clean

default: $(EXECUTABLE)

$(EXECUTABLE): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $@

run: $((EXECUTABLE)
	./$(EXECUTABLE)

clean:
	$(RM) *.o *~ $(EXECUTABLE)