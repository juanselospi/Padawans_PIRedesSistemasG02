all: ProtocolSim.out

ProtocolSim.out: ProtocolSim.cc
	g++ -g -pthread ProtocolSim.cc -o ProtocolSim.out

run: ProtocolSim.out
	./ProtocolSim.out

clean:
	rm -f *.o *.out

