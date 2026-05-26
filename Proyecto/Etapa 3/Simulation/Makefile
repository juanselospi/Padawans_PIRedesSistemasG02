all: Simulation.out

Simulation.out: Simulation.cc
	g++ -g -pthread Simulation.cc -o Simulation.out

run: Simulation.out
	./Simulation.out

clean:
	rm -f *.o *.out

