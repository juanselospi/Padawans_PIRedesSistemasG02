#include "LegoServer.h"
#include "Logger.h"

#include <sstream>
#include <iostream>
#include <thread>
#include <stdexcept>
#include <vector>

/**
 * Constructor
 */
LegoServer::LegoServer( const std::string & bindIp, int port )
   : serverSocket( 's', false ), bindIp( bindIp ), port( port ) {
}


/**
 * Read the full HTTP request headers
 */
std::string LegoServer::readHttpRequest( VSocket * client ) {

   std::string request = "";
   char buffer[1024];
   size_t bytesRead = 0;

   while ( ( bytesRead = client->Read( buffer, sizeof( buffer ) - 1 ) ) > 0 ) {
      buffer[bytesRead] = '\0';
      request += buffer;

      if ( request.find( "\r\n\r\n" ) != std::string::npos ) {
         break;
      }
   }

   return request;

}


/**
 * Extract path from first request line
 */
std::string LegoServer::getPathFromRequest( const std::string & request ) {

   std::istringstream stream( request );
   std::string method;
   std::string path;
   std::string version;

   stream >> method >> path >> version;

   if ( method != "GET" ) {
      throw std::runtime_error( "Metodo HTTP no soportado" );
   }

   if ( path.empty() || version.empty() ) {
      throw std::runtime_error( "Request HTTP invalido" );
   }

   return path;

}


/**
 * Extract query parameter from path
 */
std::string LegoServer::getQueryParam( const std::string & path, const std::string & key ) {

   size_t qpos = path.find( '?' );
   if ( qpos == std::string::npos ) {
      return "";
   }

   std::string query = path.substr( qpos + 1 );
   std::string pattern = key + "=";

   size_t start = query.find( pattern );
   if ( start == std::string::npos ) {
      return "";
   }

   start += pattern.size();
   size_t end = query.find( '&', start );

   if ( end == std::string::npos ) {
      return query.substr( start );
   }

   return query.substr( start, end - start );

}


/**
 * Build full HTTP response
 */
std::string LegoServer::buildHttpResponse( const std::string & body,
                                          const std::string & status,
                                          const std::string & contentType ) {

   std::ostringstream response;

   response << "HTTP/1.1 " << status << "\r\n";
   response << "Content-Type: " << contentType << "\r\n";
   response << "Content-Length: " << body.size() << "\r\n";
   response << "Connection: close\r\n";
   response << "\r\n";
   response << body;

   return response.str();

}


/**
 * Build /lego/index.php response
 */
std::string LegoServer::handleIndex() {

   std::cout << "Consultando el index de figuras..." << std::endl;
   Logger::log("SERVIDOR", "RESPUESTA", "FIGURES_LIST");
   std::vector<std::string> figures = fileSystem.getFigureNames();

   std::ostringstream html;

   html << "<html><body>\n";
   html << "<h1>Figuras Lego</h1>\n";
   html << "<SELECT NAME=\"figures\">\n";

   for ( const auto & fig : figures ) {
      html << "<OPTION value=\"" << fig << "\">" << fig << "</OPTION>\n";
   }

   html << "</SELECT>\n";
   html << "</body></html>\n";

   return html.str();

}


/**
 * Build /lego/list.php response
 */
std::string LegoServer::handleList( const std::string & path ) {

   std::string figure = getQueryParam( path, "figure" );
   std::string part = getQueryParam( path, "part" );

   std::vector<Piece> pieces = fileSystem.getPieces( figure, part );

   if ( pieces.empty() ) {
      Logger::log("SERVIDOR", "RESPUESTA", "FIGURE_NOT_FOUND figura=" + figure + " segmento=" + part);
   } else {
      Logger::log("SERVIDOR", "RESPUESTA", "FIGURE_FOUND figura=" + figure + " segmento=" + part);
   }

   std::ostringstream html;
   int total = 0;

   html << "<html><body>\n";
   html << "<table border=\"1\">\n";
   html << "<tr><th>Cantidad</th><th>Descripcion</th></tr>\n";

   for ( const auto & piece : pieces ) {
      html << "<tr><td>" << piece.quantity << "</td><td>" << piece.description << "</td></tr>\n";
      total += piece.quantity;
   }

   html << "<tr><td>Total de piezas para armar esta figura</td><td>" << total << "</td></tr>\n";
   html << "</table>\n";
   html << "</body></html>\n";

   return html.str();

}


/**
 * Process request and choose route
 */
std::string LegoServer::processRequest( const std::string & request ) {

   std::string path = getPathFromRequest( request );

   if ( path == "/lego/index.php" ) {
      return buildHttpResponse( handleIndex() );
   }

   if ( path.find( "/lego/list.php" ) == 0 ) {
      return buildHttpResponse( handleList( path ) );
   }

   return buildHttpResponse(
      "<html><body><h1>404 Not Found</h1></body></html>",
      "404 Not Found"
   );

}


/**
 * Worker function: attend one client
 */
void LegoServer::handleClient( VSocket * client ) {

   try {
      std::string request = readHttpRequest( client );
      std::string response = processRequest( request );
      client->Write( response.c_str() );
   } catch ( const std::exception & e ) {
      Logger::log("SERVIDOR", "ERROR", std::string("Error procesando solicitud: ") + e.what());
      try {
         std::string body =
            std::string( "<html><body><h1>500 Internal Server Error</h1><p>" ) +
            e.what() +
            "</p></body></html>";

         std::string response = buildHttpResponse( body, "500 Internal Server Error" );
         client->Write( response.c_str() );
      } catch ( ... ) {
      }
   }


   try {
      client->Close();
   } catch ( ... ) {
   }

   delete client;

}


/**
 * Main server loop
 */
void LegoServer::run() {

   serverSocket.Bind( bindIp.c_str(), port );
   serverSocket.MarkPassive( 10 );

   std::cout << "Servidor escuchando en " << bindIp << ":" << port << std::endl;

   while ( true ) {
      VSocket * client = serverSocket.AcceptConnection();

      std::thread worker( &LegoServer::handleClient, this, client );
      worker.detach();
   }

}