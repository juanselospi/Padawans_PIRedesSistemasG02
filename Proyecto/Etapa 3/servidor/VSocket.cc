/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-i
  *  Grupos: 2 y 3
  *
  ****** VSocket base class implementation
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
#include <cerrno>
#include <string>
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

   int st = -1;
   int domain;
   int sockType;

   this->IPv6 = IPv6;
   this->type = t;
   this->port = 0;
   this->sockId = -1;

   domain = IPv6 ? AF_INET6 : AF_INET;

   if ( 's' == t ) {
      sockType = SOCK_STREAM;
   } else if ( 'd' == t ) {
      sockType = SOCK_DGRAM;
   } else {
      throw std::runtime_error( "VSocket::Init, invalid socket type" );
   }

   st = socket( domain, sockType, 0 );

   if ( -1 == st ) {
      throw std::runtime_error( std::string( "VSocket::Init, socket(): " ) + strerror( errno ) );
   }

   this->sockId = st;

}


/**
  *  Class creator (constructor)
  *     use Unix socket system call
  *
  *  @param     int id: socket identifier
  *
 **/
void VSocket::Init( int id  ){

   if ( id < 0 ) {
      throw std::runtime_error( "VSocket::Init, invalid socket id" );
   }

   this->sockId = id;
   this->port = 0;
   this->type = 's';
   this->IPv6 = false;

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
   int st = -1;

   if ( this->sockId >= 0 ) {
      st = close( this->sockId );

      if ( -1 == st ) {
         throw std::runtime_error( std::string( "VSocket::Close(): " ) + strerror( errno ) );
      }

      this->sockId = -1;
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

   if ( nullptr == hostip ) {
      throw std::runtime_error( "VSocket::TryToConnect null hostip" );
   }

   if ( this->IPv6 ) {
      struct sockaddr_in6 host6;
      std::memset( &host6, 0, sizeof( host6 ) );
      host6.sin6_family = AF_INET6;
      host6.sin6_port = htons( port );

      st = inet_pton( AF_INET6, hostip, &host6.sin6_addr );
      if ( st <= 0 ) {
         throw std::runtime_error( "VSocket::TryToConnect inet_pton IPv6" );
      }

      st = connect( this->sockId, reinterpret_cast<struct sockaddr *>( &host6 ), sizeof( host6 ) );
      if ( -1 == st ) {
         throw std::runtime_error( std::string( "VSocket::TryToConnect connect IPv6: " ) + strerror( errno ) );
      }
   } else {
      struct sockaddr_in host4;
      std::memset( &host4, 0, sizeof( host4 ) );
      host4.sin_family = AF_INET;
      host4.sin_port = htons( port );

      st = inet_pton( AF_INET, hostip, &host4.sin_addr );
      if ( st <= 0 ) {
         throw std::runtime_error( "VSocket::TryToConnect inet_pton IPv4" );
      }

      st = connect( this->sockId, reinterpret_cast<struct sockaddr *>( &host4 ), sizeof( host4 ) );
      if ( -1 == st ) {
         throw std::runtime_error( std::string( "VSocket::TryToConnect connect IPv4: " ) + strerror( errno ) );
      }
   }

   this->port = port;
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
   struct addrinfo hints, *result, *rp;

   if ( nullptr == host || nullptr == service ) {
      throw std::runtime_error( "VSocket::TryToConnect null host/service" );
   }

   std::memset( &hints, 0, sizeof( hints ) );
   hints.ai_family = this->IPv6 ? AF_INET6 : AF_INET;
   hints.ai_socktype = ( 'd' == this->type ) ? SOCK_DGRAM : SOCK_STREAM;
   hints.ai_flags = 0;
   hints.ai_protocol = 0;

   st = getaddrinfo( host, service, &hints, &result );
   if ( 0 != st ) {
      throw std::runtime_error( std::string( "VSocket::TryToConnect getaddrinfo: " ) + gai_strerror( st ) );
   }

   for ( rp = result; nullptr != rp; rp = rp->ai_next ) {
      st = connect( this->sockId, rp->ai_addr, rp->ai_addrlen );
      if ( 0 == st ) {
         freeaddrinfo( result );
         return st;
      }
   }

   freeaddrinfo( result );
   throw std::runtime_error( std::string( "VSocket::TryToConnect connect: " ) + strerror( errno ) );

   return st;

}


/**
  * Bind method
  *    use "bind" Unix system call (man 3 bind) (server mode)
  *
  * @param      int port: bind a unamed socket to a port defined in sockaddr structure
  *
  *  Links the calling process to a service at port
  *
 **/
int VSocket::Bind( int port ) {
   int st = -1;
   int opt = 1;

   if ( this->sockId < 0 ) {
      throw std::runtime_error( "VSocket::Bind2026 invalid socket" );
   }

   setsockopt( this->sockId, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) );

   if ( this->IPv6 ) {
      struct sockaddr_in6 host6;
      std::memset( &host6, 0, sizeof( host6 ) );
      host6.sin6_family = AF_INET6;
      host6.sin6_addr = in6addr_any;
      host6.sin6_port = htons( port );

      st = bind( this->sockId, reinterpret_cast<struct sockaddr *>( &host6 ), sizeof( host6 ) );
   } else {
      struct sockaddr_in host4;
      std::memset( &host4, 0, sizeof( host4 ) );
      host4.sin_family = AF_INET;
      host4.sin_addr.s_addr = htonl( INADDR_ANY );
      host4.sin_port = htons( port );

      st = bind( this->sockId, reinterpret_cast<struct sockaddr *>( &host4 ), sizeof( host4 ) );
   }

   if ( -1 == st ) {
      throw std::runtime_error( std::string( "VSocket::Bind2026: " ) + strerror( errno ) );
   }

   this->port = port;
   return st;

}


/**
  * Bind method
  *    use "bind" Unix system call (man 3 bind) (server mode)
  *
  * @param      const char * ip: bind a named socket to a specific address
  * @param      int port: bind to a port defined in sockaddr structure
  *
  *  Links the calling process to a service at ip:port
  *
 **/
int VSocket::Bind( const char * ip, int port ) {
   int st = -1;
   int opt = 1;

   if ( nullptr == ip ) {
      throw std::runtime_error( "VSocket::Bind null ip" );
   }

   if ( this->sockId < 0 ) {
      throw std::runtime_error( "VSocket::Bind invalid socket" );
   }

   setsockopt( this->sockId, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) );

   if ( this->IPv6 ) {
      struct sockaddr_in6 host6;
      std::memset( &host6, 0, sizeof( host6 ) );
      host6.sin6_family = AF_INET6;
      host6.sin6_port = htons( port );

      st = inet_pton( AF_INET6, ip, &host6.sin6_addr );
      if ( st <= 0 ) {
         throw std::runtime_error( "VSocket::Bind inet_pton IPv6" );
      }

      st = bind( this->sockId, reinterpret_cast<struct sockaddr *>( &host6 ), sizeof( host6 ) );
   } else {
      struct sockaddr_in host4;
      std::memset( &host4, 0, sizeof( host4 ) );
      host4.sin_family = AF_INET;
      host4.sin_port = htons( port );

      st = inet_pton( AF_INET, ip, &host4.sin_addr );
      if ( st <= 0 ) {
         throw std::runtime_error( "VSocket::Bind inet_pton IPv4" );
      }

      st = bind( this->sockId, reinterpret_cast<struct sockaddr *>( &host4 ), sizeof( host4 ) );
   }

   if ( -1 == st ) {
      throw std::runtime_error( std::string( "VSocket::Bind: " ) + strerror( errno ) );
   }

   this->port = port;
   return st;

}


/**
  * MarkPassive method
  *    use "listen" Unix system call (man listen) (server mode)
  *
  * @param      int backlog: defines the maximum length to which the queue of pending connections for this socket may grow
  *
  *  Establish socket queue length
  *
 **/
int VSocket::MarkPassive( int backlog ) {
   int st = -1;

   st = listen( this->sockId, backlog );

   if ( -1 == st ) {
      throw std::runtime_error( std::string( "VSocket::MarkPassive: " ) + strerror( errno ) );
   }

   return st;

}


/**
  * WaitForConnection method
  *    use "accept" Unix system call (man 3 accept) (server mode)
  *
  *
  *  Waits for a peer connections, return a sockfd of the connecting peer
  *
 **/
int VSocket::WaitForConnection( void ) {
   int st = -1;

   st = accept( this->sockId, nullptr, nullptr );

   if ( -1 == st ) {
      throw std::runtime_error( std::string( "VSocket::WaitForConnection: " ) + strerror( errno ) );
   }

   return st;

}


/**
  * Shutdown method
  *    use "shutdown" Unix system call (man 3 shutdown) (server mode)
  *
  *
  *  cause all or part of a full-duplex connection on the socket associated with the file descriptor socket to be shut down
  *
 **/
int VSocket::Shutdown( int mode ) {
   int st = -1;

   st = shutdown( this->sockId, mode );

   if ( -1 == st ) {
      throw std::runtime_error( std::string( "VSocket::Shutdown: " ) + strerror( errno ) );
   }

   return st;

}


// UDP methods 2025

/**
  *  sendTo method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to send data
  *
  *  Send data to another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::sendTo( const void * buffer, size_t size, void * addr ) {
   ssize_t st = -1;

   if ( this->sockId < 0 ) {
      throw std::runtime_error( "VSocket::sendTo invalid socket" );
   }

   if ( nullptr == buffer || nullptr == addr ) {
      throw std::runtime_error( "VSocket::sendTo null buffer/addr" );
   }

   if ( this->IPv6 ) {
      st = sendto( this->sockId, buffer, size, 0,
                   reinterpret_cast<struct sockaddr *>( addr ),
                   sizeof( struct sockaddr_in6 ) );
   } else {
      st = sendto( this->sockId, buffer, size, 0,
                   reinterpret_cast<struct sockaddr *>( addr ),
                   sizeof( struct sockaddr_in ) );
   }

   if ( -1 == st ) {
      throw std::runtime_error( std::string( "VSocket::sendTo: " ) + strerror( errno ) );
   }

   return static_cast<size_t>( st );

}


/**
  *  recvFrom method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to receive from data
  *
  *  @return	size_t bytes received
  *
  *  Receive data from another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::recvFrom( void * buffer, size_t size, void * addr ) {
   ssize_t st = -1;
   socklen_t len = this->IPv6 ? sizeof( struct sockaddr_in6 ) : sizeof( struct sockaddr_in );

   if ( this->sockId < 0 ) {
      throw std::runtime_error( "VSocket::recvFrom invalid socket" );
   }

   if ( nullptr == buffer || nullptr == addr ) {
      throw std::runtime_error( "VSocket::recvFrom null buffer/addr" );
   }

   st = recvfrom( this->sockId, buffer, size, 0,
                  reinterpret_cast<struct sockaddr *>( addr ), &len );

   if ( -1 == st ) {
      throw std::runtime_error( std::string( "VSocket::recvFrom: " ) + strerror( errno ) );
   }

   return static_cast<size_t>( st );

}