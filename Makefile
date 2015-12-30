UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
ASMLIB = libaelf64.a
else
ASMLIB = libacof64.lib
endif

CXX=g++
CFLAGS=-Wall -std=c++11 -O3 -mpopcnt
	
all: testFBCSA

testFBCSA: testFBCSA.cpp libfbcsa.a libs/$(ASMLIB)
	$(CXX) $(CFLAGS) testFBCSA.cpp libfbcsa.a libs/$(ASMLIB) -o testFBCSA

libfbcsa.a: fbcsa.h fbcsa.cpp shared/common.h shared/common.cpp shared/patterns.h shared/patterns.cpp shared/timer.h shared/timer.cpp shared/sais.h shared/sais.c shared/xxhash.h shared/xxhash.c shared/hash.h shared/hash.cpp
	$(CXX) $(CFLAGS) -c fbcsa.cpp shared/common.cpp shared/patterns.cpp shared/timer.cpp shared/sais.c shared/xxhash.c shared/hash.cpp
	ar rcs libfbcsa.a fbcsa.o common.o patterns.o timer.o sais.o xxhash.o hash.o
	make cleanObjects

cleanObjects:
	rm -f *o

clean:
	rm -f *o testFBCSA libfbcsa.a