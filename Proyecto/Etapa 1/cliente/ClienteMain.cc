#include "Cliente.h"
#include <iostream>
#include <string>

int main(int argc, char * argv[]) {
    bool useSSL = (argc > 1);
    std::string input;

    do {
        Cliente cliente(useSSL);
        cliente.ejecutar();

        std::cout << "¿Desea solicitar otra figura? (s/n): ";
        std::getline(std::cin, input);

        if (input.empty()) {
            input = "n";
        }

    } while (input == "s" || input == "S");

    return 0;
}