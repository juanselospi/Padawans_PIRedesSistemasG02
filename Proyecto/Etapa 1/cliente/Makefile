# Linux Makefile
# Socket classes
#

CC  := gcc
CXX := g++
CFLAGS := -g -c
CXXFLAGS := -g -c
DEPS := Socket.h VSocket.h SSLSocket.h
TARGETS := ClienteMain.out

ALL := $(TARGETS)

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $<

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $<

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $<

OBJS := ClienteMain.o Socket.o VSocket.o SSLSocket.o HttpRequest.o HttpResponse.o Cliente.o

ClienteMain.out: $(OBJS)
	$(CXX) -g $(OBJS) -o $@ -lssl -lcrypto

clean:
	@rm -f *.o *.out

run:
	./ClienteMain.out

runssl:
	./ClienteMain.out ssl