#ifndef CLIENTE_H
#define CLIENTE_H

#include <string>
#include <vector>
#include "VSocket.h"

class Cliente {
private:
    const char * os;
    const char * osi;
    const char * ose;
    VSocket * socket;
    
    std::vector<std::string> fetchedFigures;
    void fetchFigures();

    std::string selectFigure();
    std::string selectPart();
    std::string createLink();
    void printCleanTable(const std::string& html);
    void trim(std::string &s);

public:
    Cliente();
    ~Cliente();
    void ejecutar();
};

#endif // CLIENTE_H
