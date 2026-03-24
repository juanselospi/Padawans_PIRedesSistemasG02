#include "Cliente.h"
#include <iostream>

int main(int argc, char * argv[]) {
    char continuar;
    do {
        Cliente cliente;
        cliente.ejecutar();
        
        std::cout << "¿Desea solicitar otra figura? (s/n): ";
        std::cin >> continuar;
    } while (continuar == 's' || continuar == 'S');
    
    return 0;
}