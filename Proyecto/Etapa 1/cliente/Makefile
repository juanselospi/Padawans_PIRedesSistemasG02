# Linux Makefile
# Socket classes
#

CC  := gcc
CXX := g++
CFLAGS := -g -c
CXXFLAGS := -g -c
DEPS := Socket.h
TARGETS := ClienteMain.out

ALL := $(TARGETS)

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $<

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $<

%.out: %.o Socket.o VSocket.o HttpRequest.o HttpResponse.o Cliente.o
	$(info $$^ = $^)
	$(CXX) -g $^ Socket.o VSocket.o HttpRequest.o HttpResponse.o Cliente.o -o $^

ClienteMain.out: ClienteMain.o Socket.o VSocket.o HttpRequest.o HttpResponse.o Cliente.o
	$(CXX) -g ClienteMain.o Socket.o VSocket.o HttpRequest.o HttpResponse.o Cliente.o -o $@

clean:
	@rm -f *.o *.out
