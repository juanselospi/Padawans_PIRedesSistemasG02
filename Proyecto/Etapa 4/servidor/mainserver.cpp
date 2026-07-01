#include "LegoServer.h"
#include <iostream>
#include <csignal>

int main() {

   // ignoramos SIGPIPE -> si se cae el cable mientras le respondemos a un cliente, write() no nos mata el servidor
   std::signal( SIGPIPE, SIG_IGN );

   try {
      //Here we can change the bind IP and port if needed
      LegoServer server( "0.0.0.0", 8080 );
      server.run();
   } catch ( const std::exception & e ) {
      std::cerr << "Error fatal del servidor: " << e.what() << std::endl;
      return 1;
   }

   return 0;

}
