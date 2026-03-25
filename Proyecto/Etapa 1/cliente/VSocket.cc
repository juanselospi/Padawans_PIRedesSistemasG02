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
#include <arpa/inet.h> // ntohs, htons
#include <stdexcept>   // runtime_error
#include <cstring>     // memset
#include <netdb.h>     // getaddrinfo, freeaddrinfo
#include <unistd.h>    // close
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
void VSocket::Init(char t, bool IPv6)
{

   int domain;
   int sockType;

   this->IPv6 = IPv6;
   this->type = t;
   this->port = 0;
   this->sockId = -1;

   domain = IPv6 ? AF_INET6 : AF_INET;

   if (t == 's')
   {
      sockType = SOCK_STREAM;
   }
   else if (t == 'd')
   {
      sockType = SOCK_DGRAM;
   }
   else
   {
      throw std::runtime_error("VSocket::Init, invalid socket type");
   }

   this->sockId = socket(domain, sockType, 0);

   if (this->sockId == -1)
   {
      throw std::runtime_error("VSocket::Init, socket()");
   }
}

/**
 * Class destructor
 *
 **/
VSocket::~VSocket()
{

   this->Close();
}

/**
 * Close method
 *    use Unix close system call (once opened a socket is managed like a file in Unix)
 *
 **/
void VSocket::Close()
{

   int st = 0;

   if (this->sockId >= 0)
   {
      st = close(this->sockId);

      if (st == -1)
      {
         throw std::runtime_error("VSocket::Close()");
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
int VSocket::TryToConnect(const char *hostip, int port)
{
   int st;

   if (this->IPv6)
   {
      struct sockaddr_in6 host6;
      memset(&host6, 0, sizeof(host6));
      host6.sin6_family = AF_INET6;
      host6.sin6_port = htons(port);

      st = inet_pton(AF_INET6, hostip, &host6.sin6_addr);
      if (st <= 0)
      {
         throw std::runtime_error("VSocket::TryToConnect, inet_pton() IPv6");
      }

      st = connect(this->sockId, (struct sockaddr *)&host6, sizeof(host6));
      if (st == -1)
      {
         throw std::runtime_error("VSocket::TryToConnect, connect() IPv6");
      }
   }
   else
   {
      struct sockaddr_in host4;
      memset(&host4, 0, sizeof(host4));
      host4.sin_family = AF_INET;
      host4.sin_port = htons(port);

      st = inet_pton(AF_INET, hostip, &host4.sin_addr);
      if (st <= 0)
      {
         throw std::runtime_error("VSocket::TryToConnect, inet_pton() IPv4");
      }

      st = connect(this->sockId, (struct sockaddr *)&host4, sizeof(host4));
      if (st == -1)
      {
         throw std::runtime_error("VSocket::TryToConnect, connect() IPv4");
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
int VSocket::TryToConnect(const char *host, const char *service)
{

   int st;
   struct addrinfo hints, *result, *rp;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = 0;
   hints.ai_protocol = 0;

   st = getaddrinfo(host, service, &hints, &result);
   if (st != 0)
   {
      throw std::runtime_error(gai_strerror(st));
   }

   for (rp = result; rp != nullptr; rp = rp->ai_next)
   {

      st = connect(this->sockId, rp->ai_addr, rp->ai_addrlen);

      if (st == 0)
      {
         freeaddrinfo(result);
         return st;
      }
   }

   freeaddrinfo(result);
   return st;
}