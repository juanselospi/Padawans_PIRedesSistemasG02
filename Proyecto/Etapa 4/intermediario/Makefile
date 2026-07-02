CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
COMMON_SRC = VSocket.cc Socket.cc
INTER_SRC = intermediario.cpp main.cpp
ALL_SRC = $(COMMON_SRC) $(INTER_SRC)

.PHONY: all clean run help

all: intermediario

intermediario: $(ALL_SRC) intermediario.h Logger.h Socket.h VSocket.h
	$(CXX) $(CXXFLAGS) -o intermediario $(ALL_SRC)

run: intermediario
	# estos parametros son: ip del inter, puerto clientes, ip server, puerto server, ip broadcast, ip semilla que usamos solo en pruebas
	./intermediario 172.16.123.35 8081 172.16.123.38 8080 192.168.1.255


run_local: intermediario
	./intermediario 0.0.0.0 8081 auto 8080 255.255.255.255 127.0.0.1 2026

run_ip:
	./intermediario 0.0.0.0 8081 auto 8080 255.255.255.255 10.69.199.11 2026

clean:
	rm -f intermediario *.o
	