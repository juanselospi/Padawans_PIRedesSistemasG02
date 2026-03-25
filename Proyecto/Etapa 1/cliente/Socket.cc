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

#include <sys/socket.h> // sockaddr_in
#include <arpa/inet.h>  // ntohs
#include <unistd.h>     // write, read
#include <cstring>
#include <stdexcept>
#include <stdio.h>  // printf
#include "Socket.h" // Derived class

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
Socket::Socket(char t, bool IPv6)
{

   this->Init(t, IPv6);
}

/**
 *  Class destructor
 *
 *  @param     int id: socket descriptor
 *
 **/
Socket::~Socket()
{
   this->Close();
}

/**
 * Connect method
 *   use "TryToConnect" in base class
 *
 * @param      char * host: host address in dot notation, example "10.1.166.62"
 * @param      int port: process address, example 80
 *
 **/
int Socket::Connect(const char *hostip, int port)
{
   return this->TryToConnect(hostip, port);
}

/**
 * Connect method
 *   use "TryToConnect" in base class
 *
 * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
 * @param      char * service: process address, example "http"
 *
 **/
int Socket::Connect(const char *host, const char *service)
{
   return this->TryToConnect(host, service);
}

/**
 * Read method
 *   use "read" Unix system call (man 3 read)
 *
 * @param      void * buffer: buffer to store data read from socket
 * @param      int size: buffer capacity, read will stop if buffer is full
 *
 **/
size_t Socket::Read(void *buffer, size_t size)
{
   if (this->sockId == -1)
      throw std::runtime_error("Socket is not able to read, need to initialize");

   if (buffer == nullptr)
      throw std::runtime_error("Socket::Read(), null buffer");

   if (size == 0)
      return 0;

   ssize_t bytesRead = read(this->sockId, buffer, size);

   if (bytesRead < 0)
   {
      throw std::runtime_error(
          "Read error: " + std::string(strerror(errno)));
   }

   return static_cast<size_t>(bytesRead);
}

/**
 * Write method
 *   use "write" Unix system call (man 3 write)
 *
 * @param      void * buffer: buffer to store data write to socket
 * @param      size_t size: buffer capacity, number of bytes to write
 *
 **/
size_t Socket::Write(const void *buffer, size_t size)
{
   if (this->sockId == -1)
      throw std::runtime_error("Socket can not write, initialize it first");

   if (buffer == nullptr)
      throw std::runtime_error("Socket::Write(), null buffer");

   if (size == 0)
      return 0;

   size_t totalWritten = 0;

   while (totalWritten < size)
   {
      ssize_t dataWritten = write(
          this->sockId,
          static_cast<const char *>(buffer) + totalWritten,
          size - totalWritten);

      if (dataWritten < 0)
      {
         if (errno == EINTR)
            continue; // reintentar

         throw std::runtime_error(
             "Write error: " + std::string(strerror(errno)));
      }

      if (dataWritten == 0)
      {
         throw std::runtime_error("Write returned 0, connection closed?");
      }

      totalWritten += static_cast<size_t>(dataWritten);
   }

   return totalWritten;
}

/**
 * Write method
 *   use "write" Unix system call (man 3 write)
 *
 * @param      char * text: text to write to socket
 *
 **/
size_t Socket::Write(const char *text)
{

   if (text == nullptr)
   {
      throw std::runtime_error("Socket::Write( char * )");
   }

   return this->Write(text, strlen(text));
}