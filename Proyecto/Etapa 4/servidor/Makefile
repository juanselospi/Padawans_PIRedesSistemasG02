# Compilador
CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread -IFilesystem

# Archivos comunes
COMMON = VSocket.cc Socket.cc

# Filesystem real
FS_SRC = Filesystem/filesystem.cc Filesystem/disk.cc
FS_OBJ = Filesystem/filesystem.o Filesystem/disk.o

# Servidor
SERVER_SRC = LegoServer.cpp mainserver.cpp FileSystem.cpp $(COMMON)
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
SERVER_OBJ := $(SERVER_OBJ:.cc=.o) $(FS_OBJ)

# Targets
all: servidor

servidor: $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o server $(SERVER_OBJ)

# Herramienta para cargar datos iniciales
seed:
	$(MAKE) -C Filesystem seed

# Regla para compilar .cpp
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla para compilar .cc
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar
clean:
	rm -f *.o Filesystem/*.o server Filesystem/lego_data.bin

# Ejecutar rápido
run: servidor
	./server

.PHONY: all clean run seed
