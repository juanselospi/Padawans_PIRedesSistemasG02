#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include <map>

class HttpResponse {
public:
    HttpResponse();
    explicit HttpResponse(const std::string& rawResponse);

    void parse(const std::string& rawResponse);

    int getStatusCode() const;
    std::string getStatusMessage() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;
    std::string getHttpVersion() const;

private:
    std::string httpVersion;    // HTTP/1.1
    int statusCode;             // 200, 404, etc.
    std::string statusMessage;  // OK, Not Found

    std::map<std::string, std::string> headers;
    std::string body;
};

#endif