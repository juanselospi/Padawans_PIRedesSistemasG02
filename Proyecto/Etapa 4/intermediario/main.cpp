#include "intermediario.h"
#include <iostream>
#include <cstdlib>
#include <csignal>

int main(int argc, char * argv[]) {
    // ignoramos SIGPIPE -> si un peer cierra mientras le escribimos, write() no nos mata el proceso
    std::signal(SIGPIPE, SIG_IGN);

    std::string bindIp = "0.0.0.0";
    int clientPort = 8081;
    std::string figServerIp = "auto"; // descubrir nuestro servidor por UDP
    int figServerPort = 8080;
    std::string broadcastAddr = "255.255.255.255"; // broadcast para descubrir intermediarios remotos
    std::string seedPeerIp = "";
    int seedPeerPort = 2026;

    if(argc >= 2) bindIp = argv[1];
    if(argc >= 3) clientPort = std::atoi(argv[2]);
    if(argc >= 4) figServerIp = argv[3];
    if(argc >= 5) figServerPort = std::atoi(argv[4]);
    if(argc >= 6) broadcastAddr = argv[5];
    if(argc >= 7) seedPeerIp = argv[6];
    if(argc >= 8) seedPeerPort = std::atoi(argv[7]);

    bool autoServer = (figServerIp == "auto" || figServerIp.empty());

    std::cout << " - - - Servidor Intermediario Lego - - - \n"
              << "  Clientes en: " << bindIp << ":" << clientPort << "\n"
              << "  Servidor de figuras: "
              << (autoServer ? std::string("autodescubrimiento UDP ") + std::to_string(8090)
                             : figServerIp + ":" + std::to_string(figServerPort))
              << "\n"
              << "  UDP JOIN en: " << bindIp << ":" << clientPort << "\n"
              << "  TCP Peers en: " << bindIp << ":" << clientPort << "\n"
              << "  Descubrir peers: broadcast " << broadcastAddr << " (UDP 2027)\n"
              << "  Semilla peer: "
              << (seedPeerIp.empty() ? std::string("(ninguna)")
                                        : seedPeerIp + ":" + std::to_string(seedPeerPort))
              << "\n";

    try
    {
        IntermediaryServer server(bindIp, clientPort, figServerIp, figServerPort, broadcastAddr, seedPeerIp, seedPeerPort);
        server.run();

    } catch(const std::exception & e) {

        std::cerr << "Error fatal del intermediario: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
