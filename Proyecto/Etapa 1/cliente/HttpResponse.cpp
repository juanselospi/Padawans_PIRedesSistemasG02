#include "HttpResponse.h"
#include <sstream>
#include <stdexcept>

HttpResponse::HttpResponse()
    : httpVersion(""), statusCode(0), statusMessage(""), body("")
{
}

HttpResponse::HttpResponse(const std::string &rawResponse)
    : httpVersion(""), statusCode(0), statusMessage(""), body("")
{
    parse(rawResponse);
}

void HttpResponse::parse(const std::string &rawResponse)
{
    headers.clear();
    body.clear();
    httpVersion.clear();
    statusCode = 0;
    statusMessage.clear();

    // Separar los headers del body
    std::size_t headerEnd = rawResponse.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        throw std::runtime_error("Respuesta HTTP invalida: no se encontro fin de encabezados");
    }

    // Parte de headres creada desde el inicio hasta el final de los headers
    std::string headerPart = rawResponse.substr(0, headerEnd);

    //Parte justo despues de los headers, despues del \r\n\r\n
    body = rawResponse.substr(headerEnd + 4);
    // Verlo como lineas
    std::istringstream stream(headerPart);
    std::string line;

    // Primero ver que no este vacio y leer el status line, el HTTP/1.1 200 OK
    if (!std::getline(stream, line))
    {
        throw std::runtime_error("Respuesta HTTP invalida: vacia");
    }

    // Quitar '\r' si quedo al final
    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }

    std::istringstream statusLine(line);
    statusLine >> httpVersion;
    statusLine >> statusCode;
    std::getline(statusLine, statusMessage);

    if (!statusMessage.empty() && statusMessage[0] == ' ')
    {
        statusMessage.erase(0, 1);
    }

    // Leer headers restantes
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (line.empty())
        {
            continue;
        }

        std::size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            if (!value.empty() && value[0] == ' ')
            {
                value.erase(0, 1);
            }

            headers[key] = value;
        }
    }
}

int HttpResponse::getStatusCode() const
{
    return statusCode;
}

std::string HttpResponse::getStatusMessage() const
{
    return statusMessage;
}

std::string HttpResponse::getHeader(const std::string &key) const
{
    auto it = headers.find(key);
    if (it != headers.end())
    {
        return it->second;
    }
    return "";
}

std::string HttpResponse::getBody() const
{
    return body;
}

std::string HttpResponse::getHttpVersion() const
{
    return httpVersion;
}