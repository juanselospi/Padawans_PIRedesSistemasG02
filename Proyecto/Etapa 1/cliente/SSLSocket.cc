/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-i
  *  Grupos: 2 y 3
  *
  *  SSL Socket class implementation
  *
  * (Fedora version)
  *
 **/
 
// SSL includes
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <stdexcept>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "SSLSocket.h"
#include "Socket.h"

/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket( bool IPv6 ) {

   this->Init( 's', IPv6 );

   this->Context = nullptr;
   this->BIO = nullptr;

   this->InitSSL();   // Initializes to client context

}


/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool IPv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket( char * certFileName, char * keyFileName, bool IPv6 ) {

   this->Init( 's', IPv6 );

   this->Context = nullptr;
   this->BIO = nullptr;

   this->InitSSL( true );                  // server context
   this->LoadCertificates( certFileName, keyFileName );

}


/**
  *  Class constructor
  *
  *  @param     int id: socket descriptor
  *
 **/
SSLSocket::SSLSocket( int id ) {

   this->Init( id );

   this->Context = nullptr;
   this->BIO = nullptr;
}


/**
  * Class destructor
  *
 **/
SSLSocket::~SSLSocket() {

   // SSL destroy
   if ( nullptr != this->BIO ) {
      SSL_shutdown( reinterpret_cast<SSL *>( this->BIO ) );
      SSL_free( reinterpret_cast<SSL *>( this->BIO ) );
      this->BIO = nullptr;
   }

   if ( nullptr != this->Context ) {
      SSL_CTX_free( reinterpret_cast<SSL_CTX *>( this->Context ) );
      this->Context = nullptr;
   }

   this->Close();
}


/**
  *  InitSSL
  *     use SSL_new with a defined context
  *
  *  Create a SSL object
  *
 **/
void SSLSocket::InitSSL( bool serverContext ) {
   SSL * ssl = nullptr;

   this->InitContext( serverContext );

   ssl = SSL_new( reinterpret_cast<SSL_CTX *>( this->Context ) );
   if ( nullptr == ssl ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::InitSSL( bool )" );
   }

   this->BIO = ssl;
}


/**
  *  InitContext
  *     use SSL_library_init, OpenSSL_add_all_algorithms, SSL_load_error_strings, TLS_server_method, SSL_CTX_new
  *
  *  Creates a new SSL server context to start encrypted comunications, this context is stored in class instance
  *
 **/
void SSLSocket::InitContext( bool serverContext ) {
   const SSL_METHOD * method = nullptr;
   SSL_CTX * context = nullptr;

   SSL_library_init();
   OpenSSL_add_all_algorithms();
   SSL_load_error_strings();

   if ( serverContext ) {
      method = TLS_server_method();
   } else {
      method = TLS_client_method();
   }

   if ( nullptr == method ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::InitContext( bool )" );
   }

   context = SSL_CTX_new( method );
   if ( nullptr == context ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::InitContext( bool )" );
   }

   this->Context = context;
}


   //luego el contexto

/**
 *  Load certificates
 *    verify and load certificates
 *
 *  @param	const char * certFileName, file containing certificate
 *  @param	const char * keyFileName, file containing keys
 *
 **/
void SSLSocket::LoadCertificates( const char * certFileName, const char * keyFileName ) {

   SSL_CTX * ctx = reinterpret_cast<SSL_CTX *>( this->Context );

   if ( nullptr == ctx ) {
      throw std::runtime_error( "SSLSocket::LoadCertificates(): null context" );
   }

   // Load certificate
   if ( SSL_CTX_use_certificate_file( ctx, certFileName, SSL_FILETYPE_PEM ) <= 0 ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::LoadCertificates(): certificate" );
   }

   // Load private key
   if ( SSL_CTX_use_PrivateKey_file( ctx, keyFileName, SSL_FILETYPE_PEM ) <= 0 ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::LoadCertificates(): private key" );
   }

   // Verify private key
   if ( !SSL_CTX_check_private_key( ctx ) ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::LoadCertificates(): key does not match certificate" );
   }
}
 

/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	int port, service number
 *
 **/
int SSLSocket::Connect( const char * hostName, int port ) {
   int st;

   st = this->TryToConnect( hostName, port );   // Establish a non ssl connection first
   if ( st == -1 ) {
      throw std::runtime_error( "SSLSocket::Connect( const char *, int )" );
   }

   SSL_set_fd( reinterpret_cast<SSL *>( this->BIO ), this->sockId );

   st = SSL_connect( reinterpret_cast<SSL *>( this->BIO ) );
   if ( st <= 0 ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::Connect( const char *, int )" );
   }

   return st;
}


/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	char * service, service name
 *
 **/
int SSLSocket::Connect( const char * host, const char * service ) {
   int st;

   st = this->TryToConnect( host, service );
   if ( st == -1 ) {
      throw std::runtime_error( "SSLSocket::Connect( const char *, const char * )" );
   }

   SSL_set_fd( reinterpret_cast<SSL *>( this->BIO ), this->sockId );

   st = SSL_connect( reinterpret_cast<SSL *>( this->BIO ) );
   if ( st <= 0 ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::Connect( const char *, const char * )" );
   }

   return st;
}


/**
  *  Read
  *     use SSL_read to read data from an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity read
  *
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Read(void * buffer, size_t size) {
    SSL * ssl = reinterpret_cast<SSL *>(this->BIO);

    int st = SSL_read(ssl, buffer, static_cast<int>(size));
    
    if (st > 0) {
        return static_cast<size_t>(st);
    }
    
    int err = SSL_get_error(ssl, st);
    switch (err) {
        case SSL_ERROR_ZERO_RETURN:
            // El peer cerro la conexion TLS de forma normal
            return 0;

        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            // No es error fatal; en este proyecto podemos tratarlo como fin temporal
            return 0;

        case SSL_ERROR_SYSCALL:
            // Cierre abrupto o error del sistema
            if (st == 0) {
                return 0;
            }
            ERR_print_errors_fp(stderr);
            throw std::runtime_error("SSLSocket::Read(): SSL_ERROR_SYSCALL");

        default:
            ERR_print_errors_fp(stderr);
            throw std::runtime_error("SSLSocket::Read(): SSL_read failed");
    }
}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Writes data to a secure channel
  *
 **/
size_t SSLSocket::Write( const char * string ) {
   int st = -1;

   st = SSL_write( reinterpret_cast<SSL *>( this->BIO ),
                   string,
                   static_cast<int>( std::strlen( string ) ) );

   if ( st <= 0 ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::Write( const char * )" );
   }

   return static_cast<size_t>( st );
}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	const void * buffer to store data to write
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Write( const void * buffer, size_t size ) {
   int st = -1;

   st = SSL_write( reinterpret_cast<SSL *>( this->BIO ),
                   buffer,
                   static_cast<int>( size ) );

   if ( st <= 0 ) {
      ERR_print_errors_fp( stderr );
      throw std::runtime_error( "SSLSocket::Write( void *, size_t )" );
   }

   return static_cast<size_t>( st );
}


/**
 *   Show SSL certificates
 *
 **/
void SSLSocket::ShowCerts() {
   X509 * cert;
   char * line;

   cert = SSL_get_peer_certificate( reinterpret_cast<SSL *>( this->BIO ) );  // Get certificates (if available)
   if ( nullptr != cert ) {
      printf( "Server certificates:\n" );
      line = X509_NAME_oneline( X509_get_subject_name( cert ), 0, 0 );
      printf( "Subject: %s\n", line );
      free( line );
      line = X509_NAME_oneline( X509_get_issuer_name( cert ), 0, 0 );
      printf( "Issuer: %s\n", line );
      free( line );
      X509_free( cert );
   } else {
      printf( "No certificates.\n" );
   }

}


/**
 *   Return the name of the currently used cipher
 *
 **/
const char * SSLSocket::GetCipher() {

   return SSL_get_cipher( reinterpret_cast<SSL *>( this->BIO ) );

}