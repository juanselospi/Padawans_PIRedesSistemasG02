#ifndef FileSystem_h
#define FileSystem_h

#include <string>
#include <vector>
#include "Piece.h"

class FileSystem {
   public:
      FileSystem();

      std::vector<std::string> getFigureNames() const;
      std::vector<Piece> getPieces( const std::string &, const std::string & ) const;
};

#endif