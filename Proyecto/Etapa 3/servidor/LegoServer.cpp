#include "LegoServer.h"
#include "Logger.h"

#include <sstream>
#include <iostream>
#include <thread>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sys/socket.h>

/**
 * Constructor
 */
LegoServer::LegoServer(const std::string &bindIp, int port)
    : serverSocket('s', false), bindIp(bindIp), port(port)
{
}

/**
 * Lee una solicitud del socket.
 *
 * El servidor soporta dos formatos:
 * 1. HTTP normal, por ejemplo GET /lego/index.php HTTP/1.1
 * 2. Protocolo Intragrupal, por ejemplo P/R/dir o P/G/figura
 *
 * Si el mensaje empieza con P/, se deja de leer después del primer bloque
 * para no quedarse esperando encabezados HTTP.
 */
std::string LegoServer::readHttpRequest(VSocket *client)
{
   std::string request = "";
   char buffer[1024];
   size_t bytesRead = 0;

   while ((bytesRead = client->Read(buffer, sizeof(buffer) - 1)) > 0)
   {
      buffer[bytesRead] = '\0';
      request += buffer;

      if (isProtocolMessage(request))
      {
         break;
      }

      if (request.find("\r\n\r\n") != std::string::npos)
      {
         break;
      }
   }

   return request;
}

/**
 * Extrae la ruta de una solicitud HTTP.
 */
std::string LegoServer::getPathFromRequest(const std::string &request)
{
   std::istringstream stream(request);
   std::string method;
   std::string path;
   std::string version;

   stream >> method >> path >> version;

   if (method != "GET")
   {
      throw std::runtime_error("HTTP method not supported");
   }

   if (path.empty() || version.empty())
   {
      throw std::runtime_error("Invalid HTTP request");
   }

   return path;
}

/**
 * Extrae un parametro del query string.
 */
std::string LegoServer::getQueryParam(const std::string &path, const std::string &key)
{
   size_t qpos = path.find('?');
   if (qpos == std::string::npos)
   {
      return "";
   }

   std::string query = path.substr(qpos + 1);
   std::string pattern = key + "=";

   size_t start = query.find(pattern);
   if (start == std::string::npos)
   {
      return "";
   }

   start += pattern.size();
   size_t end = query.find('&', start);

   if (end == std::string::npos)
   {
      return query.substr(start);
   }

   return query.substr(start, end - start);
}

/**
 * Construye una respuesta HTTP completa.
 */
std::string LegoServer::buildHttpResponse(const std::string &body,
                                          const std::string &status,
                                          const std::string &contentType)
{
   std::ostringstream response;

   response << "HTTP/1.1 " << status << "\r\n";
   response << "Content-Type: " << contentType << "\r\n";
   response << "Content-Length: " << body.size() << "\r\n";
   response << "Connection: close\r\n";
   response << "\r\n";
   response << body;

   return response.str();
}

std::string LegoServer::handleBitacora()
{
   std::ifstream file("bitacora.log");

   std::ostringstream html;
   html << "<html><head>";
   html << "<meta charset=\"UTF-8\">";
   html << "<title>Log</title>";
   html << "<style>";
   html << "body{font-family:Arial;margin:30px;background:#f4f4f4;}";
   html << "h1{color:#333;}";
   html << "table{border-collapse:collapse;width:100%;background:white;}";
   html << "th,td{border:1px solid #ccc;padding:8px;text-align:left;}";
   html << "th{background:#222;color:white;}";
   html << "tr:nth-child(even){background:#f2f2f2;}";
   html << ".error{color:red;font-weight:bold;}";
   html << ".ok{color:green;font-weight:bold;}";
   html << "</style>";
   html << "</head><body>";

   html << "<h1>Server Log</h1>";

   if (!file.is_open())
   {
      html << "<p class=\"error\">bitacora.log does not exist or could not be opened.</p>";
   }
   else
   {
      html << "<table>";
      html << "<tr><th>Entry</th></tr>";

      std::string line;
      while (std::getline(file, line))
      {
         html << "<tr><td>" << line << "</td></tr>";
      }

      html << "</table>";
   }

   html << "</body></html>";

   return html.str();
}

std::string LegoServer::handleBitacoraCliente()
{
   std::ifstream file("../cliente/bitacora.log");

   std::ostringstream html;
   html << "<html><head><meta charset=\"UTF-8\">";
   html << "<title>Client Log</title>";
   html << "<style>";
   html << "body{font-family:Arial;margin:30px;background:#f4f4f4;}";
   html << "table{border-collapse:collapse;width:100%;background:white;}";
   html << "th,td{border:1px solid #ccc;padding:8px;text-align:left;}";
   html << "th{background:#222;color:white;}";
   html << "</style></head><body>";

   html << "<h1>Client Log</h1>";

   if (!file.is_open())
   {
      html << "<p>Could not open ../cliente/bitacora.log</p>";
   }
   else
   {
      html << "<table><tr><th>Entry</th></tr>";
      std::string line;
      while (std::getline(file, line))
      {
         html << "<tr><td>" << line << "</td></tr>";
      }
      html << "</table>";
   }

   html << "</body></html>";
   return html.str();
}

std::string LegoServer::handleIndex()
{
   std::cout << "Fetching figures index..." << std::endl;
   Logger::log("SERVER", "RESPONSE", "FIGURES_LIST");

   std::vector<std::string> figures = fileSystem.getFigureNames();

   std::ostringstream html;

   html << "<html><head>";
   html << "<meta charset=\"UTF-8\">";
   html << "<title>Lego Figures</title>";
   html << "<style>";
   html << "body{font-family:Arial;margin:30px;background:#f4f4f4;color:#222;}";
   html << "h1{font-size:38px;margin-bottom:10px;}";
   html << ".card{background:white;padding:25px;border-radius:12px;box-shadow:0 2px 8px rgba(0,0,0,.12);}";
   html << "select{width:100%;padding:12px;font-size:18px;border-radius:8px;border:1px solid #ccc;margin-top:15px;}";
   html << ".fig{padding:10px;margin:8px 0;background:#e8e8e8;border-radius:6px;font-size:18px;}";
   html << ".fig a{text-decoration:none;color:#222;font-weight:bold;}";
   html << ".fig a:hover{color:#007BFF;}";
   html << "</style>";
   html << "</head><body>";

   html << "<div class=\"card\">";
   html << "<h1>Available Lego Figures</h1>";

   html << "<SELECT NAME=\"figures\">";

   if (figures.empty())
   {
      html << "<OPTION value=\"None\">No figures available</OPTION>";
   }
   else
   {
      for (const auto &fig : figures)
      {
         html << "<OPTION value=\"" << fig << "\">" << fig << "</OPTION>";
      }
   }

   html << "</SELECT>";

   if (!figures.empty())
   {
      for (const auto &fig : figures)
      {
         html << "<div class=\"fig\">";
         html << "<a href=\"/lego/list.php?figure=" << fig << "&part=1\">"
              << fig << " - Part 1</a><br>";
         html << "<a href=\"/lego/list.php?figure=" << fig << "&part=2\">"
              << fig << " - Part 2</a>";
         html << "</div>";
      }
   }

   html << "</div>";
   html << "</body></html>";

   return html.str();
}

/**
 * Construye la respuesta HTTP para /lego/list.php.
 */
std::string LegoServer::handleList(const std::string &path)
{
   std::string figure = getQueryParam(path, "figure");
   std::string part = getQueryParam(path, "part");

   std::vector<Piece> pieces = fileSystem.getPieces(figure, part);

   if (pieces.empty())
   {
      Logger::log("SERVER", "RESPONSE", "FIGURE_NOT_FOUND figure=" + figure + " segment=" + part);
   }
   else
   {
      Logger::log("SERVER", "RESPONSE", "FIGURE_FOUND figure=" + figure + " segment=" + part);
   }

   std::ostringstream html;
   int total = 0;

   html << "<html><head>";
   html << "<meta charset=\"UTF-8\">";
   html << "<title>Lego Pieces</title>";
   html << "<style>";
   html << "body{font-family:Arial;margin:30px;background:#f4f4f4;color:#222;}";
   html << "h1{font-size:38px;margin-bottom:5px;}";
   html << "h2{font-size:20px;font-weight:normal;color:#555;margin-top:0;}";
   html << ".card{background:white;padding:25px;border-radius:12px;box-shadow:0 2px 8px rgba(0,0,0,.12);}";
   html << "table{border-collapse:collapse;width:100%;margin-top:20px;background:white;}";
   html << "th,td{border:1px solid #ccc;padding:12px;text-align:left;font-size:18px;}";
   html << "th{background:#222;color:white;}";
   html << "tr:nth-child(even){background:#f2f2f2;}";
   html << ".total{font-weight:bold;background:#e8e8e8;}";
   html << ".empty{color:#b00020;font-weight:bold;font-size:18px;}";
   html << "</style>";
   html << "</head><body>";

   html << "<div class=\"card\">";
   html << "<h1>Pieces List</h1>";
   html << "<h2>Figure: " << figure << " | Segment: " << part << "</h2>";

   if (pieces.empty())
   {
      html << "<p class=\"empty\">No pieces found for this figure.</p>";
   }

   html << "<table border=\"1\">";
   html << "<tr><th>CANTIDAD</th><th>DESCRIPTION</th></tr>";

   for (const auto &piece : pieces)
   {
      html << "<tr>";
      html << "<td>" << piece.quantity << "</td>";
      html << "<td>" << piece.description << "</td>";
      html << "</tr>";
      total += piece.quantity;
   }

   html << "<tr class=\"total\">";
   html << "<td>Total pieces to build this figure</td>";
   html << "<td>" << total << "</td>";
   html << "</tr>";

   html << "</table>";
   html << "</div>";
   html << "</body></html>";

   return html.str();
}

/**
 * Limpia saltos de linea, espacios finales y caracteres nulos.
 */
std::string LegoServer::cleanProtocolRawMessage(const std::string &raw) const
{
   std::string result = raw;

   result.erase(std::remove(result.begin(), result.end(), '\0'), result.end());

   while (!result.empty() &&
          (result.back() == '\n' || result.back() == '\r' ||
           result.back() == ' ' || result.back() == '\t'))
   {
      result.pop_back();
   }

   return result;
}

/**
 * Un mensaje del protocolo intragrupal empieza con P/.
 *
 * Ejemplos:
 * P/R/dir
 * P/G/carro
 * P/Q/
 */
bool LegoServer::isProtocolMessage(const std::string &raw) const
{
   std::string message = cleanProtocolRawMessage(raw);
   return message.size() >= 2 && message[0] == 'P' && message[1] == '/';
}

/**
 * Parsea el protocolo intragrupal.
 *
 * Formato:
 * P/COMANDO/mensaje
 */
LegoServer::ProtocolMessage LegoServer::parseProtocolMessage(const std::string &raw) const
{
   std::string message = cleanProtocolRawMessage(raw);

   if (message.size() < 4 || message[0] != 'P' || message[1] != '/' || message[3] != '/')
   {
      throw std::runtime_error("Mensaje de protocolo invalido. Formato esperado: P/C/mensaje");
   }

   ProtocolMessage parsed;
   parsed.protocol = message[0];
   parsed.command = message[2];
   parsed.message = message.substr(4);

   return parsed;
}

/**
 * Construye una respuesta del protocolo intragrupal.
 *
 * Ejemplo:
 * buildProtocolMessage('D', "fig1,fig2") -> P/D/fig1,fig2
 */
std::string LegoServer::buildProtocolMessage(char command, const std::string &message) const
{
   std::ostringstream response;
   response << "P/" << command << "/" << message;
   return response.str();
}

/**
 * Maneja solicitud de directorio.
 *
 * Entrada:
 * P/R/dir
 *
 * Salida:
 * P/D/fig1,fig2,fig3
 */
std::string LegoServer::handleProtocolDirectory()
{
   std::vector<std::string> figures = fileSystem.getFigureNames();
   std::ostringstream data;

   for (size_t i = 0; i < figures.size(); ++i)
   {
      if (i > 0)
      {
         data << ",";
      }

      data << figures[i];
   }

   Logger::log("SERVER", "PROTO_RESPONSE", "DIRECTORY figures=" + data.str());

   return buildProtocolMessage('D', data.str());
}

/**
 * Maneja solicitud de figura.
 *
 * Formatos aceptados:
 *
 * P/G/nombreFigura  devuelve parte 1 + parte 2
 * P/G/nombreFigura:1  devuelve solo parte 1
 * P/G/nombreFigura:2   devuelve solo parte 2
 *
 * Salida:
 * P/D/cantidad|pieza,cantidad|pieza
 */
std::string LegoServer::handleProtocolFigure(const std::string &figureRequest)
{
   if (figureRequest.empty())
   {
      Logger::log("SERVER", "PROTO_RESPONSE", "FIGURE_REQUEST_EMPTY");
      return buildProtocolMessage('D', "404");
   }

   std::string figure = figureRequest;
   std::string requestedPart = "";

   size_t separatorPosition = figureRequest.find(':');

   if (separatorPosition != std::string::npos)
   {
      figure = figureRequest.substr(0, separatorPosition);
      requestedPart = figureRequest.substr(separatorPosition + 1);
   }

   if (figure.empty())
   {
      Logger::log("SERVER", "PROTO_RESPONSE", "FIGURE_NAME_EMPTY");
      return buildProtocolMessage('D', "404");
   }

   std::vector<Piece> pieces;

   if (requestedPart == "1")
   {
      pieces = fileSystem.getPieces(figure, "1");
   }
   else if (requestedPart == "2")
   {
      pieces = fileSystem.getPieces(figure, "2");
   }
   else if (requestedPart.empty())
   {
      std::vector<Piece> piecesPart1 = fileSystem.getPieces(figure, "1");
      std::vector<Piece> piecesPart2 = fileSystem.getPieces(figure, "2");

      pieces.insert(pieces.end(), piecesPart1.begin(), piecesPart1.end());
      pieces.insert(pieces.end(), piecesPart2.begin(), piecesPart2.end());
   }
   else
   {
      Logger::log("SERVER", "PROTO_RESPONSE", "INVALID_PART figure=" + figure + " part=" + requestedPart);
      return buildProtocolMessage('D', "400");
   }

   if (pieces.empty())
   {
      Logger::log("SERVER", "PROTO_RESPONSE", "FIGURE_NOT_FOUND figure=" + figure + " part=" + requestedPart);
      return buildProtocolMessage('D', "404");
   }

   std::ostringstream data;

   for (size_t i = 0; i < pieces.size(); ++i)
   {
      if (i > 0)
      {
         data << ",";
      }

      data << pieces[i].quantity << "|" << pieces[i].description;
   }

   if (requestedPart.empty())
   {
      Logger::log("SERVER", "PROTO_RESPONSE", "FIGURE_FOUND figure=" + figure + " complete");
   }
   else
   {
      Logger::log("SERVER", "PROTO_RESPONSE", "FIGURE_FOUND figure=" + figure + " part=" + requestedPart);
   }

   return buildProtocolMessage('D', data.str());
}

/**
 * Procesa mensajes del protocolo intragrupal.
 */
std::string LegoServer::processProtocolRequest(const std::string &request)
{
   ProtocolMessage msg = parseProtocolMessage(request);

   switch (msg.command)
   {
      case 'R':
         /*
          * REQUEST:
          * P/R/dir
          */
         if (msg.message == "dir" || msg.message.empty())
         {
            return handleProtocolDirectory();
         }

         Logger::log("SERVER", "PROTO_RESPONSE", "BAD_REQUEST message=" + msg.message);
         return buildProtocolMessage('D', "400");

      case 'G':
         /*
          * GET:
          * P/G/nombreFigura
          * P/G/nombreFigura:1
          * P/G/nombreFigura:2
          */
         return handleProtocolFigure(msg.message);

      case 'Q':
         /*
          * QUIT:
          * P/Q/
          */
         Logger::log("SERVER", "PROTO_RESPONSE", "QUIT");
         return buildProtocolMessage('A', "");

      case 'C':
         /*
          * CONNECT:
          * P/C/ip:puerto
          */
         Logger::log("SERVER", "PROTO_RESPONSE", "CONNECT from=" + msg.message);
         return buildProtocolMessage('A', "");

      default:
         Logger::log("SERVER", "PROTO_RESPONSE", std::string("UNKNOWN_COMMAND command=") + msg.command);
         return buildProtocolMessage('D', "400");
   }
}

/**
 * Procesa la solicitud.
 *
 * Si empieza con P/, se procesa como protocolo intragrupal.
 * Si no, se mantiene el comportamiento HTTP original.
 */
std::string LegoServer::processRequest(const std::string &request)
{
   if (isProtocolMessage(request))
   {
      return processProtocolRequest(request);
   }

   bool isNachos = request.find("User-Agent: nachos") != std::string::npos;
   std::string path = getPathFromRequest(request);

   if (path == "/lego/index.php")
   {
      return isNachos ? buildHttpResponse(handleIndexNachos())
                      : buildHttpResponse(handleIndex());
   }

   if (path.find("/lego/list.php") == 0)
   {
      return isNachos ? buildHttpResponse(handleListNachos(path))
                      : buildHttpResponse(handleList(path));
   }

   if (path == "/log")
   {
      return buildHttpResponse(handleBitacora());
   }

   if (path == "/log/client")
   {
      return buildHttpResponse(handleBitacoraCliente());
   }

   return buildHttpResponse(
       "<html><body><h1>404 Not Found</h1></body></html>",
       "404 Not Found");
}

/**
 * Atiende un cliente.
 */
void LegoServer::handleClient(VSocket *client)
{
   try
   {
      std::string request = readHttpRequest(client);
      std::string response = processRequest(request);

      client->Write(response.c_str());
      client->Shutdown(SHUT_WR);
   }
   catch (const std::exception &e)
   {
      Logger::log("SERVIDOR", "ERROR", std::string("Error procesando solicitud: ") + e.what());

      try
      {
         std::string body =
             std::string("<html><body><h1>500 Internal Server Error</h1><p>") +
             e.what() +
             "</p></body></html>";

         std::string response = buildHttpResponse(body, "500 Internal Server Error");
         client->Write(response.c_str());
      }
      catch (...)
      {
      }
   }

   try
   {
      client->Close();
   }
   catch (...)
   {
   }

   delete client;
}

/**
 * Respuesta simple para cliente NachOS.
 */
std::string LegoServer::handleIndexNachos()
{
   std::vector<std::string> figures = fileSystem.getFigureNames();

   std::string result = "";

   for (const auto &fig : figures)
   {
      result += fig + "\n";
   }

   return result;
}

/**
 * Respuesta simple para cliente NachOS.
 */
std::string LegoServer::handleListNachos(const std::string &path)
{
   std::string figure = getQueryParam(path, "figure");
   std::string part = getQueryParam(path, "part");

   std::vector<Piece> pieces = fileSystem.getPieces(figure, part);

   std::string result = "";

   for (const auto &piece : pieces)
   {
      result += std::to_string(piece.quantity) + "|" + piece.description + "\n";
   }

   return result;
}

/**
 * Ciclo principal del servidor.
 */
void LegoServer::run()
{
   serverSocket.Bind(bindIp.c_str(), port);
   serverSocket.MarkPassive(10);

   std::cout << "Servidor escuchando en " << bindIp << ":" << port << std::endl;

   while (true)
   {
      VSocket *client = serverSocket.AcceptConnection();

      std::thread worker(&LegoServer::handleClient, this, client);
      worker.detach();
   }
}