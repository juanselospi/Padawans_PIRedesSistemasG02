#ifndef LegoServer_h
#define LegoServer_h

#include <string>
#include "Socket.h"
#include "FileSystem.h"
#include <sys/socket.h>

class LegoServer {
   private:
      Socket serverSocket;
      FileSystem fileSystem;
      std::string bindIp;
      int port;

      struct ProtocolMessage {
         char protocol;
         char command;
         std::string message;
      };

      std::string readHttpRequest( VSocket * );
      std::string processRequest( const std::string & );
      std::string buildHttpResponse( const std::string &,
                                     const std::string & = "200 OK",
                                     const std::string & = "text/html; charset=UTF-8" );

      bool isProtocolMessage( const std::string & ) const;
      ProtocolMessage parseProtocolMessage( const std::string & ) const;
      std::string buildProtocolMessage( char, const std::string & = "" ) const;
      std::string processProtocolRequest( const std::string & );
      std::string handleProtocolDirectory();
      std::string handleProtocolFigure( const std::string & );
      std::string cleanProtocolRawMessage( const std::string & ) const;

      std::string getPathFromRequest( const std::string & );
      std::string getQueryParam( const std::string &, const std::string & );

      std::string handleIndex();
      std::string handleList( const std::string & );

      void handleClient( VSocket * );
      void discoveryResponder();   // responde "LEGO_SERVER <puerto>" a "LEGO_DISCOVER" por UDP
      std::string handleBitacora();
      std::string handleBitacoraCliente();
      std::string handleListNachos( const std::string &path );
      std::string handleIndexNachos();

   public:
      LegoServer( const std::string &, int );
      void run();
};

#endif