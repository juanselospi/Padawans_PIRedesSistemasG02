/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-i
  *  Grupos: 2 y 3
  *
  *  ******   Socket class implementation
  *
  * (Fedora version)
  *
 **/

#include <sys/socket.h>         // sockaddr_in
#include <arpa/inet.h>          // ntohs
#include <unistd.h>		// write, read
#include <cstring>
#include <stdexcept>
#include <stdio.h>		// printf
#include <cerrno>
#include <string>

#include "Socket.h"		// Derived class

/**
  *  Class constructor
  *     use Unix socket system call
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
Socket::Socket( char t, bool IPv6 ){

   this->Init( t, IPv6 );      // Call base class constructor

}


/**
  *  Class constructor
  *     use an already accepted socket descriptor
  *
  *  @param     int id: socket descriptor
  *
 **/
Socket::Socket( int id ) {

   this->Init( id );

}


/**
  *  Class destructor
  *
  *  @param     int id: socket descriptor
  *
 **/
Socket::~Socket() {

}


/**
  * Connect method
  *   use "TryToConnect" in base class
  *
  * @param      char * host: host address in dot notation, example "10.1.166.62"
  * @param      int port: process address, example 80
  *
 **/
int Socket::Connect( const char * hostip, int port ) {

   return this->TryToConnect( hostip, port );

}


/**
  * Connect method
  *   use "TryToConnect" in base class
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int Socket::Connect( const char *host, const char *service ) {

   return this->TryToConnect( host, service );

}


/**
  * Read method
  *   use "read" Unix system call (man 3 read)
  *
  * @param      void * buffer: buffer to store data read from socket
  * @param      int size: buffer capacity, read will stop if buffer is full
  *
 **/
size_t Socket::Read( void * buffer, size_t size ) {

   ssize_t st = -1;

   if ( this->sockId < 0 ) {
      throw std::runtime_error( "Socket::Read( void *, size_t ) invalid socket" );
   }

   if ( nullptr == buffer ) {
      throw std::runtime_error( "Socket::Read( void *, size_t ) null buffer" );
   }

   if ( 0 == size ) {
      return 0;
   }

   st = read( this->sockId, buffer, size );

   if ( -1 == st ) {
      throw std::runtime_error( std::string( "Socket::Read( void *, size_t ): " ) + strerror( errno ) );
   }

   return static_cast<size_t>( st );

}


/**
  * Write method
  *   use "write" Unix system call (man 3 write)
  *
  * @param      void * buffer: buffer to store data write to socket
  * @param      size_t size: buffer capacity, number of bytes to write
  *
 **/
size_t Socket::Write( const void * buffer, size_t size ) {

   size_t total = 0;
   ssize_t st = -1;

   if ( this->sockId < 0 ) {
      throw std::runtime_error( "Socket::Write( void *, size_t ) invalid socket" );
   }

   if ( nullptr == buffer ) {
      throw std::runtime_error( "Socket::Write( void *, size_t ) null buffer" );
   }

   if ( 0 == size ) {
      return 0;
   }

   while ( total < size ) {
      st = write( this->sockId,
                  static_cast<const char *>( buffer ) + total,
                  size - total );

      if ( -1 == st ) {
         if ( EINTR == errno ) {
            continue;
         }
         throw std::runtime_error( std::string( "Socket::Write( void *, size_t ): " ) + strerror( errno ) );
      }

      if ( 0 == st ) {
         throw std::runtime_error( "Socket::Write( void *, size_t ) connection closed" );
      }

      total += static_cast<size_t>( st );
   }

   return total;

}


/**
  * Write method
  *   use "write" Unix system call (man 3 write)
  *
  * @param      char * text: text to write to socket
  *
 **/
size_t Socket::Write( const char * text ) {

   if ( nullptr == text ) {
      throw std::runtime_error( "Socket::Write( char * ) null text" );
   }

   return this->Write( text, std::strlen( text ) );

}


/**
  * Bind method
  *   use base class bind with explicit IP
  *
  * @param      const char * ip: IP address to bind, example "10.84.166.62"
  * @param      int port: process address, example 8080
  *
 **/
int Socket::Bind( const char * ip, int port ) {

   return VSocket::Bind( ip, port );

}


/**
  * AcceptConnection method
  *    use base class to accept connections
  *
  *  @returns   a new class instance
  *
  *  Waits for a new connection to service (TCP mode: stream)
  *
 **/
VSocket * Socket::AcceptConnection(){
   int id;
   VSocket * peer;

   id = this->WaitForConnection();

   peer = new Socket( id );

   return peer;

}