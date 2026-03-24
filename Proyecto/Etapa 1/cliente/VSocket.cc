/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-i
  *  Grupos: 2 y 3
  *
  *******   VSocket base class implementation
  *
  * (Fedora version)
  *
 **/

#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>		// getaddrinfo, freeaddrinfo
#include <unistd.h>		// close
/*
#include <cstddef>
#include <cstdio>

//#include <sys/types.h>
*/
#include "VSocket.h"


/**
  *  Class creator (constructor)
  *     use Unix socket system call
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
void VSocket::Init( char t, bool IPv6 ){

   this->type = t;
   this->IPv6 = IPv6;

   int domain = IPv6 ? AF_INET6 : AF_INET;
   int sockType = (t == 's') ? SOCK_STREAM : SOCK_DGRAM;

   int st = socket( domain, sockType, 0 );

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Init, socket" );
   }

   this->sockId = st;
}


/**
  * Class destructor
  *
 **/
VSocket::~VSocket() {

   this->Close();

}


/**
  * Close method
  *    use Unix close system call (once opened a socket is managed like a file in Unix)
  *
 **/
void VSocket::Close(){
   int st = close( this->sockId );

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Close()" );
   }

}


/**
  * TryToConnect method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dot notation, example "10.84.166.62"
  * @param      int port: process address, example 80
  *
 **/
int VSocket::TryToConnect( const char * hostip, int port ) {

   int st = -1;

   if ( this->IPv6 ) {
      struct sockaddr_in6 host6;
      memset( (char *) &host6, 0, sizeof( host6 ) );
      host6.sin6_family = AF_INET6;
      st = inet_pton( AF_INET6, hostip, &host6.sin6_addr );
      if ( 1 != st ) {
         throw std::runtime_error( "VSocket::TryToConnect, inet_pton" );
      }
      host6.sin6_port = htons( port );
      st = connect( this->sockId, (struct sockaddr *) &host6, sizeof( host6 ) );
   } else {
      struct sockaddr_in host4;
      memset( (char *) &host4, 0, sizeof( host4 ) );
      host4.sin_family = AF_INET;
      st = inet_pton( AF_INET, hostip, &host4.sin_addr );
      if ( 1 != st ) {
         throw std::runtime_error( "VSocket::TryToConnect, inet_pton" );
      }
      host4.sin_port = htons( port );
      st = connect( this->sockId, (struct sockaddr *) &host4, sizeof( host4 ) );
   }

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::TryToConnect, connect" );
   }

   return st;

}


/**
  * TryToConnect method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int VSocket::TryToConnect( const char *host, const char *service ) {
   int st = -1;

   struct addrinfo hints;
   struct addrinfo *result, *rp;

   memset( &hints, 0, sizeof( hints ) );
   hints.ai_family = this->IPv6 ? AF_INET6 : AF_INET;
   hints.ai_socktype = (this->type == 's') ? SOCK_STREAM : SOCK_DGRAM;
   hints.ai_flags = 0;
   hints.ai_protocol = 0;

   st = getaddrinfo( host, service, &hints, &result );
   if ( st != 0 ) {
      throw std::runtime_error( "VSocket::TryToConnect, getaddrinfo" );
   }

   for ( rp = result; rp != NULL; rp = rp->ai_next ) {
      st = connect( this->sockId, rp->ai_addr, rp->ai_addrlen );
      if ( st != -1 ) {
         break; // Success
      }
   }

   freeaddrinfo( result );

   if ( rp == NULL ) {
      throw std::runtime_error( "VSocket::TryToConnect, connect" );
   }

   return st;

}

