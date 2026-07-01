#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <mutex>
#include <iomanip>
#include <sstream>

class Logger {
public:
    static void log(const std::string& entidad, const std::string& tipo, const std::string& mensaje) {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);

        std::time_t now = std::time(nullptr);
        std::tm* ltm = std::localtime(&now);
        
        std::ostringstream timeStr;
        timeStr << std::setfill('0') << std::setw(2) << ltm->tm_mday << "/"
                << std::setfill('0') << std::setw(2) << (ltm->tm_mon + 1) << "/"
                << (1900 + ltm->tm_year) << " "
                << std::setfill('0') << std::setw(2) << ltm->tm_hour << ":"
                << std::setfill('0') << std::setw(2) << ltm->tm_min;

        std::string logLine = "[" + entidad + "] [" + timeStr.str() + "] [" + tipo + "] " + mensaje;

        // Log to console
        std::cout << logLine << std::endl;

        // Log to file
        std::ofstream file("bitacora.log", std::ios::app);
        if (file.is_open()) {
            file << logLine << std::endl;
        }
    }
};

#endif
