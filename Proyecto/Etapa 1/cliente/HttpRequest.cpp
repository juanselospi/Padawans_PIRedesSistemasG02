#include "HttpRequest.h"

HttpRequest::HttpRequest()
    : method("GET"), path("/"), host("")
{
}

void HttpRequest::setMethod(const std::string &method)
{
    this->method = method;
}

void HttpRequest::setPath(const std::string &path)
{
    this->path = path;
}

void HttpRequest::setHost(const std::string &host)
{
    this->host = host;
}

void HttpRequest::addHeader(const std::string &key, const std::string &value)
{
    this->headers[key] = value;
}

std::string HttpRequest::toString() const
{
    std::string request;

    // Línea inicial de la petición HTTP
    // Ejemplo: GET /lego/list.php?figure=Horse&part=1 HTTP/1.1
    request += method + " " + path + " HTTP/1.1\r\n";

    // Host 
    // Ejemplo: Host: os.ecci.ucr.ac.cr
    request += "Host: " + host + "\r\n";

    // Headers extra
    for (const auto &header : headers)
    {
        // Para no repetir Host si ya fue agregado aparte
        if (header.first != "Host")
        {
            request += header.first + ": " + header.second + "\r\n";
        }
    }

    // Fin del encabezado HTTP
    request += "\r\n";

    return request;
}