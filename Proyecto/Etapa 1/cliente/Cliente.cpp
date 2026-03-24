#include "Cliente.h"
#include "Socket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cctype>


// Constructor de la clase Cliente
Cliente::Cliente() {
    os = "http://os.ecci.ucr.ac.cr/";
    osi = "10.84.166.62";
    ose = "163.178.104.62";
    socket = new Socket('s');
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
void Cliente::printCleanTable(const std::string& html) {
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
std::string Cliente::createLink() {
   std::string figure = selectFigure();
   std::string part = selectPart();
   
   HttpRequest request;
   request.setMethod("GET");
   std::string path = std::string("/lego/list.php?figure=") + figure + "&part=" + part;
   request.setPath(path);
   request.setHost("redes.ecci");
   
   return request.toString();
}

// Metodo que permite obtener las figuras del servidor
void Cliente::fetchFigures() {
    Socket tempSocket('s');
    tempSocket.Connect(ose, 80);
    
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
        // En caso de error de parseo se omitirá o se pondrá lista por defecto
    }
    
    // Si la lista quedo vacía luego del intento, usar la lista predeterminada
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
       fetchFigures();
   }
   
   printf("Seleccione el numero de figura:\n");
   for (size_t i = 0; i < fetchedFigures.size(); ++i) {
       printf("%zu. %s\n", i + 1, fetchedFigures[i].c_str());
   }

   int option;
   if (scanf("%d", &option) != 1) {
       // Limpiar el buffer de entrada en caso de que escriban texto
       int c;
       while ((c = getchar()) != '\n' && c != EOF);
       printf("Entrada invalida\n");
       return selectFigure();
   }

   if (option < 1 || option > (int)fetchedFigures.size()) {
      printf("Opcion invalida\n");
      //retry
      return selectFigure();
   }

   return fetchedFigures[option - 1];
}


// Metodo que permite seleccionar la parte de la figura
std::string Cliente::selectPart() {
   std::string part;
   printf("Seleccione la parte de la figura: \n"
   "1. Primera mitad\n"
   "2. Segunda mitad\n");

   int option;
   if (scanf("%d", &option) != 1) {
       int c;
       while ((c = getchar()) != '\n' && c != EOF);
       printf("Entrada invalida\n");
       return selectPart();
   }

   if (option < 1 || option > 2) {
      printf("Opcion invalida\n");
      //retry
      return selectPart();
   }

   switch (option) {
      case 1: part = "1"; break;
      case 2: part = "2"; break;
   }
   return part;
}


// Metodo que permite ejecutar el cliente
void Cliente::ejecutar() {
   std::string reqStr = createLink();
   socket->Connect(ose, 80);   // usar "osi" en la ECCI, "ose" en sus casas
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
       printCleanTable(httpResponse.getBody());
   } catch (const std::exception& e) {
       std::cerr << "Error parseando respuesta HTTP: " << e.what() << "\n";
       printCleanTable(response);
   }
}
