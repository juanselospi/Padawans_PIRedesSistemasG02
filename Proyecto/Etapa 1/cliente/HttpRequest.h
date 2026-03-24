#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>
#include <map>

class HttpRequest {
public:
    HttpRequest();

    void setMethod(const std::string& method);
    void setPath(const std::string& path);
    void setHost(const std::string& host);
    void addHeader(const std::string& key, const std::string& value);

    std::string toString() const;

private:
    std::string method;   // GET, POST, etc.
    std::string path;     // /lego/list.php?figure=Horse&part=1
    std::string host;     // os.ecci.ucr.ac.cr
    std::map<std::string, std::string> headers;
};

#endif