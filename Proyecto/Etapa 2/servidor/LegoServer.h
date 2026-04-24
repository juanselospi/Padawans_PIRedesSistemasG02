#ifndef LegoServer_h
#define LegoServer_h

#include <string>
#include "Socket.h"
#include "FileSystem.h"

class LegoServer {
   private:
      Socket serverSocket;
      FileSystem fileSystem;
      std::string bindIp;
      int port;

      std::string readHttpRequest( VSocket * );
      std::string processRequest( const std::string & );
      std::string buildHttpResponse( const std::string &,
                                     const std::string & = "200 OK",
                                     const std::string & = "text/html; charset=UTF-8" );

      std::string getPathFromRequest( const std::string & );
      std::string getQueryParam( const std::string &, const std::string & );

      std::string handleIndex();
      std::string handleList( const std::string & );

      void handleClient( VSocket * );

   public:
      LegoServer( const std::string &, int );
      void run();
};

#endif