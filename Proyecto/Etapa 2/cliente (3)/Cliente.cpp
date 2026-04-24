#include "Cliente.h"
#include "Socket.h"
#include "SSLSocket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Logger.h"


#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cctype>


// Constructor de la clase Cliente
Cliente::Cliente(bool useSSL) {
    this->useSSL = useSSL;
    this->port = 8080;
    osi = "127.0.0.1";
    ose = "192.168.1.106";
    if (useSSL) {
        os = "https://os.ecci.ucr.ac.cr/";
        socket = new SSLSocket();   // IPv4 SSL
        std::cout << "[Modo: HTTPS con SSL - Puerto " << port << "]\n";
    } else {
        os = "http://os.ecci.ucr.ac.cr/";
        socket = new Socket('s');   // IPv4 normal
        std::cout << "[Modo: HTTP sin SSL - Puerto " << port << "]\n";
    }
}


// Destructor de la clase Cliente
Cliente::~Cliente() {
    delete socket;
}


// Metodo que permite eliminar los espacios en blanco de una cadena
void Cliente::trim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}


// Metodo que permite imprimir la tabla de forma limpia
void Cliente::FIGURE_PARTS(const std::string& html) {
    std::string upperHtml = html;
    for (char &c : upperHtml) c = std::toupper((unsigned char)c);

    size_t tableSearchStart = 0;
    size_t piecesTableStart = std::string::npos;
    
    //buscar la tabla que contiene la palabra "CANTIDAD"
    while ((tableSearchStart = upperHtml.find("<TABLE", tableSearchStart)) != std::string::npos) {
        size_t nextTable = upperHtml.find("</TABLE>", tableSearchStart);
        if (nextTable == std::string::npos) nextTable = upperHtml.length();
        
        std::string tableContent = upperHtml.substr(tableSearchStart, nextTable - tableSearchStart);
        if (tableContent.find("CANTIDAD") != std::string::npos) {
            piecesTableStart = tableSearchStart;
            break;
        }
        tableSearchStart = nextTable;
    }

    //si no se encuentra la tabla, imprimir el html sin procesar
    if (piecesTableStart == std::string::npos) {
        size_t bodyStart = html.find("\r\n\r\n");
        if (bodyStart != std::string::npos) {
            std::cout << html.substr(bodyStart + 4) << std::endl;
        } else {
            std::cout << html << std::endl;
        }
        return;
    }

    size_t tableStart = piecesTableStart;

    size_t tableEnd = upperHtml.find("</TABLE>", tableStart);
    if (tableEnd == std::string::npos) tableEnd = html.length();
    
    //imprimir la tabla de forma limpia
    std::cout << "\n=== Lista de Piezas ===\n";
    size_t trPos = tableStart;
    while ((trPos = upperHtml.find("<TR", trPos)) != std::string::npos && trPos < tableEnd) {
        size_t trEnd = upperHtml.find("</TR>", trPos);
        if (trEnd == std::string::npos || trEnd > tableEnd) trEnd = tableEnd;
        
        std::string rowText = "";
        
        size_t cellPos = trPos;
        bool firstCell = true;
        while (cellPos < trEnd) {
            size_t tdSearch = upperHtml.find("<TD", cellPos);
            size_t thSearch = upperHtml.find("<TH", cellPos);
            
            size_t startCell = std::min(tdSearch, thSearch);
            if (startCell == std::string::npos || startCell >= trEnd) break;
            
            size_t endCell = upperHtml.find(">", startCell);
            if (endCell == std::string::npos || endCell >= trEnd) break;
            
            size_t closeTd = upperHtml.find("</TD>", endCell);
            size_t closeTh = upperHtml.find("</TH>", endCell);
            
            size_t closeCell = std::min(closeTd, closeTh);
            if (closeCell == std::string::npos || closeCell > trEnd) closeCell = trEnd;
            
            std::string cellContent = html.substr(endCell + 1, closeCell - (endCell + 1));
            
            std::string cleanCell = "";
            bool inTag = false;
            for (char c : cellContent) {
                if (c == '<') inTag = true;
                else if (c == '>') inTag = false;
                else if (!inTag) cleanCell += c;
            }
            
            trim(cleanCell);
            
            if (!cleanCell.empty() && cleanCell != "Imagen") {
                if (!firstCell) rowText += " | ";
                rowText += cleanCell;
                firstCell = false;
            }
            
            cellPos = closeCell + 4;
        }
        
        if (!rowText.empty()) {
            std::cout << rowText << std::endl;
        }
        
        trPos = trEnd + 4;
    }
    std::cout << "=======================\n\n";
}

// Metodo que permite crear el link para obtener las figuras del servidor
std::string Cliente::GET_FIGURE() {
    currentFigure = selectFigure();
    currentPart = selectPart();

    Logger::log("CLIENTE", "SOLICITUD", "GET_FIGURE figura=" + currentFigure + " segmento=" + currentPart);

    HttpRequest request;

    request.setMethod("GET");
    std::string path = std::string("/lego/list.php?figure=") + currentFigure + "&part=" + currentPart;
    request.setPath(path);
    request.setHost("redes.ecci");

    return request.toString();
}

// Metodo que permite obtener las figuras del servidor
void Cliente::LIST_FIGURES() {
    // int port = useSSL ? 443 : 80;
    // Usamos un socket del mismo tipo que el principal
    if (useSSL) {
        Logger::log("CLIENTE", "SOLICITUD", "LIST_FIGURES");
        SSLSocket tempSocket;

        try {
            tempSocket.Connect(ose, port);
        } catch (const std::exception& e) {
            Logger::log("CLIENTE", "ERROR", std::string("Error de conexion: ") + e.what());
            std::cerr << "Error de conexion: " << e.what() << "\n";
            // Usamos la lista predeterminada si no hay conexion
            fetchedFigures = {
                "Horse", "blacksheep", "dragon", "duck", "elephant",
                "fish", "giraffe", "horse", "lion", "monkey", "orchid",
                "penguin", "q", "roadrunner", "shark", "squid", "swan",
                "turtle", "whitesheep"
            };
            return;
        }

        HttpRequest request;
        request.setMethod("GET");
        request.setPath("/lego/index.php");
        request.setHost("redes.ecci");

        std::string reqStr = request.toString();
        tempSocket.Write(reqStr.c_str());

        std::string response = "";
        char a[512];
        size_t bytesRead;
        while ((bytesRead = tempSocket.Read(a, sizeof(a) - 1)) > 0) {
            a[bytesRead] = '\0';
            response += a;
        }

        try {
            HttpResponse httpResponse(response);
            std::string body = httpResponse.getBody();
            size_t selectPos = body.find("<SELECT NAME=\"figures\"");
            if (selectPos != std::string::npos) {
                size_t selectEnd = body.find("</SELECT>", selectPos);
                size_t optionPos = body.find("<OPTION", selectPos);
                while (optionPos != std::string::npos && optionPos < selectEnd) {
                    size_t valStart = body.find("value=\"", optionPos);
                    if (valStart != std::string::npos && valStart < selectEnd) {
                        valStart += 7;
                        size_t valEnd = body.find("\"", valStart);
                        if (valEnd != std::string::npos && valEnd < selectEnd) {
                            std::string fig = body.substr(valStart, valEnd - valStart);
                            if (fig != "None" && fig != "0") {
                                fetchedFigures.push_back(fig);
                            }
                        }
                    }
                    optionPos = body.find("<OPTION", optionPos + 7);
                }
            }
        } catch (...) {}

        if (fetchedFigures.empty()) {
            fetchedFigures = {
                "Horse", "blacksheep", "dragon", "duck", "elephant",
                "fish", "giraffe", "horse", "lion", "monkey", "orchid",
                "penguin", "q", "roadrunner", "shark", "squid", "swan",
                "turtle", "whitesheep"
            };
        }
        return;
    }

    // Sin SSL:
    Logger::log("CLIENTE", "SOLICITUD", "LIST_FIGURES");
    Socket tempSocket('s');

    try {
        tempSocket.Connect(ose, port);
    } catch (const std::exception& e) {
        Logger::log("CLIENTE", "ERROR", std::string("Error de conexion: ") + e.what());
        std::cerr << "Error de conexion: " << e.what() << "\n";
        // Usamos la lista predeterminada si no hay conexion
        fetchedFigures = {
            "Horse", "blacksheep", "dragon", "duck", "elephant", 
            "fish", "giraffe", "horse", "lion", "monkey", "orchid", 
            "penguin", "q", "roadrunner", "shark", "squid", "swan", 
            "turtle", "whitesheep"
        };
        return;
    }
    
    HttpRequest request;
    request.setMethod("GET");
    request.setPath("/lego/index.php");
    request.setHost("redes.ecci");
    
    std::string reqStr = request.toString();
    tempSocket.Write(reqStr.c_str());
    
    std::string response = "";
    char a[512];
    size_t bytesRead;

    while ( (bytesRead = tempSocket.Read(a, sizeof(a) - 1)) > 0 ) {
        a[bytesRead] = '\0';
        response += a;
    }
    
    try {
        HttpResponse httpResponse(response);
        std::string body = httpResponse.getBody();
        
        size_t selectPos = body.find("<SELECT NAME=\"figures\"");
        if (selectPos != std::string::npos) {
            size_t selectEnd = body.find("</SELECT>", selectPos);
            size_t optionPos = body.find("<OPTION", selectPos);
            while (optionPos != std::string::npos && optionPos < selectEnd) {
                size_t valStart = body.find("value=\"", optionPos);
                if (valStart != std::string::npos && valStart < selectEnd) {
                    valStart += 7;
                    size_t valEnd = body.find("\"", valStart);
                    if (valEnd != std::string::npos && valEnd < selectEnd) {
                        std::string fig = body.substr(valStart, valEnd - valStart);
                        // Filtramos None y otras opciones invalidas si es que hay
                        if (fig != "None" && fig != "0") {
                            fetchedFigures.push_back(fig);
                        }
                    }
                }
                optionPos = body.find("<OPTION", optionPos + 7);
            }
        }
    } catch (...) {
        // En caso de error de parseo se omitira o se pondra lista por defecto
    }
    
    // Si la lista quedo vacia luego del intento, usar la lista predeterminada
    if (fetchedFigures.empty()) {
        fetchedFigures = {
            "Horse", "blacksheep", "dragon", "duck", "elephant", 
            "fish", "giraffe", "horse", "lion", "monkey", "orchid", 
            "penguin", "q", "roadrunner", "shark", "squid", "swan", 
            "turtle", "whitesheep"
        };
    }
}

// Metodo que permite seleccionar la figura
std::string Cliente::selectFigure() {
    if (fetchedFigures.empty()) {
        LIST_FIGURES();
    }

    printf("Seleccione el numero de figura:\n");
    for (size_t i = 0; i < fetchedFigures.size(); ++i) {
        printf("%zu. %s\n", i + 1, fetchedFigures[i].c_str());
    }

    std::string input;
    std::getline(std::cin, input);

   // Entrada vacia
    if (input.empty()) {
        printf("Entrada vacia, intente de nuevo\n");
        return selectFigure();
    }

    int option;
    try {
        option = std::stoi(input);
    } catch (...) {
        printf("Entrada invalida\n");
        return selectFigure();
    }

    if (option < 1 || option > (int)fetchedFigures.size()) {
        printf("Opcion invalida\n");
        return selectFigure();
    }

    return fetchedFigures[option - 1];
}


// Metodo que permite seleccionar la parte de la figura
std::string Cliente::selectPart() {
    printf("Seleccione la parte de la figura: \n"
            "1. Primera mitad\n"
            "2. Segunda mitad\n");

    std::string input;
    std::getline(std::cin, input);

   // Entrada vacia
    if (input.empty()) {
        printf("Entrada vacia, intente de nuevo\n");
        return selectPart();
    }

    int option;
    try {
        option = std::stoi(input);
    } catch (...) {
        printf("Entrada invalida\n");
        return selectPart();
    }

    if (option < 1 || option > 2) {
        printf("Opcion invalida\n");
        return selectPart();
    }

    switch (option) {
        case 1: return "1";
        case 2: return "2";
    }

    return ""; // por seguridad (nunca deberia llegar aqui)
}


// Metodo que permite ejecutar el cliente
void Cliente::ejecutar() {
    std::string reqStr = GET_FIGURE();
    // int port = useSSL ? 443 : 80;
    try {
        socket->Connect(ose, port);   // usar "osi" en la ECCI, "ose" en  casa
    } catch (const std::exception& e) {
        Logger::log("CLIENTE", "ERROR", std::string("Error de conexion: ") + e.what());
        std::cerr << "Error de conexion: " << e.what() << "\n";
        return;
    }

    socket->Write(reqStr.c_str());

    std::string response = "";
    char a[512];
    size_t bytesRead;

    while ( (bytesRead = socket->Read(a, sizeof(a) - 1)) > 0 ) {
        a[bytesRead] = '\0';
        response += a;
    }

    try {
        HttpResponse httpResponse(response);
        Logger::log("CLIENTE", "RESPUESTA", "RETURN_FIGURE figura=" + currentFigure + " segmento=" + currentPart);
        FIGURE_PARTS(httpResponse.getBody());

    } catch (const std::exception& e) {
        std::string errorMsg = std::string("Error parseando respuesta HTTP: ") + e.what();
        Logger::log("CLIENTE", "ERROR", errorMsg);
        std::cerr << errorMsg << "\n";
        FIGURE_PARTS(response);
    }

}
