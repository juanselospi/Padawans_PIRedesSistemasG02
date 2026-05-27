#include "LegoServer.h"
#include <iostream>

int main() {

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
