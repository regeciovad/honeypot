# Makefile pro projekt do predmetu ISA, rok 2015/2016
# Varianta: FTP/SSH Honeypot
# Autor: Dominika Regeciova, xregec00

CXX=g++
RM=rm -rf
CXXFLAGS=-std=c++11 -g -Wall
SRCS=fakesrv.cpp
HDRS=fakesrv.hpp
EXECUTABLE=fakesrv

.PHONY: clean

default: $(EXECUTABLE)

$(EXECUTABLE): $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $@

run: $((EXECUTABLE)
	./$(EXECUTABLE)

clean:
	$(RM) *.o *~ $(EXECUTABLE)