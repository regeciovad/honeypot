# Makefile for ISA course project, 2015/2016
# Variant: FTP/SSH Honeypot
# Author: Dominika Regeciova, xregec00

CXX=g++
RM=rm -rf
CXXFLAGS=-std=c++11 -g -Wall -pedantic -pthread
SRCS=fakesrv.cpp ftp.cpp logging.cpp
HDRS=fakesrv.hpp ftp.hpp logging.hpp
EXECUTABLE=fakesrv

.PHONY: clean

default: $(EXECUTABLE)

$(EXECUTABLE): $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $@

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	$(RM) *.o *~ $(EXECUTABLE)