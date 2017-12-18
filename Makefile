UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
ASMLIB = libaelf64.a
else
ASMLIB = libacof64.lib
endif

CXX=g++
CFLAGS=-Wall -faligned-new -std=c++11 -O3 -mpopcnt
	
all: countFBCSA locateFBCSA extractFBCSA

countFBCSA: test/countFBCSA.cpp libfbcsa.a libs/$(ASMLIB)
	$(CXX) $(CFLAGS) test/countFBCSA.cpp libfbcsa.a libs/$(ASMLIB) -o test/countFBCSA
	
locateFBCSA: test/locateFBCSA.cpp libfbcsa.a libs/$(ASMLIB)
	$(CXX) $(CFLAGS) test/locateFBCSA.cpp libfbcsa.a libs/$(ASMLIB) -o test/locateFBCSA
	
extractFBCSA: test/extractFBCSA.cpp libfbcsa.a libs/$(ASMLIB)
	$(CXX) $(CFLAGS) test/extractFBCSA.cpp libfbcsa.a libs/$(ASMLIB) -o test/extractFBCSA

libfbcsa.a: fbcsa.hpp shared/common.hpp shared/patterns.hpp shared/timer.hpp shared/sais.h shared/sais.c shared/xxhash.h shared/xxhash.c shared/hash.hpp
	$(CXX) $(CFLAGS) -c shared/sais.c shared/xxhash.c
	ar rcs libfbcsa.a fbcsa.hpp sais.o xxhash.o shared/common.hpp shared/patterns.hpp shared/hash.hpp shared/timer.hpp
	make cleanObjects

cleanObjects:
	rm -f *o

clean:
	rm -f *o test/countFBCSA test/extractFBCSA test/locateFBCSA libfbcsa.a