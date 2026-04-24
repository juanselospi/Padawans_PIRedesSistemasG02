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
    int port;
    bool useSSL;
    VSocket * socket;
    
    std::vector<std::string> fetchedFigures;
    std::string currentFigure;
    std::string currentPart;

    void LIST_FIGURES();

    std::string selectFigure();
    std::string selectPart();
    std::string GET_FIGURE();
    void FIGURE_PARTS(const std::string& html);
    void trim(std::string &s);

public:
    Cliente(bool useSSL = false);
    ~Cliente();
    void ejecutar();
};

#endif // CLIENTE_H
