#include "URLReceiver.h"


#include "../frontier/frontier.h"

string UrlReceiver::parseUrls(const char *buffer) {

    // delimit by new line

    auto urls = string(buffer);
    std::cout << "Receiving urls" << std::endl;  


    string substring = urls.substr(0);
    int pos = substring.find("\n");

    vector<string> recvUrls;


    while (pos != -1) {
        const string& url = substring.substr(0, pos);    

        if (!url.empty()) {
            recvUrls.push_back(urls);
        }

        substring = substring.substr(pos + 1);
        pos = substring.find("\n");
    }
    
    if (frontierPtr) {
        frontierPtr->insert(recvUrls);
    }

    return substring;
}

void UrlReceiver::stopListening() {
    listenFlag = false;
}

UrlReceiver::UrlReceiver( const int id_in, const uint16_t port_in, ThreadSafeFrontier* frontierPtr) :  frontierPtr(frontierPtr), server() {
    listenFlag = true;
    
    this->id = id_in;
    this->port = port_in;


    std::cout << "init url receiver " << this->id << " port "<< this->port <<std::endl;

}

void UrlReceiver::listener() {
    std::string ipEnv = std::string("NODE_IP") + std::to_string(id);
    const char* ip = std::getenv(ipEnv.c_str());
    std::string ipStr(ip);
    


    try {
        server.createServer(ipStr, port);
        std::cout << "URLReceiver started listening on port: " << port << std::endl; 
    } catch (const std::exception &e) {
        std::cerr << "Error creating server: " << e.what() << std::endl;
        throw;
    }

    
    while (listenFlag) {
        int new_socket = server.acceptConnection();

        if (new_socket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            sleep(1);
            continue;  // Retry accepting connections
        }



        uint32_t len;
        ssize_t received = recv(new_socket, &len, sizeof(len), MSG_WAITALL);
        if (received <= 0) {
            std::cerr << "Error receiving length" << std::endl;
            server.closeConnection();
            continue;  // Retry accepting connections
        }
        
        len = ntohl(len);
        vector<char> buffer;
        buffer.resize(len, '\0');
        
        received = recv(new_socket, buffer.data(), len, MSG_WAITALL);

        if (received < 0) {
            std::cerr << "Error reading data" << std::endl;
        } 

        parseUrls(buffer.data());

        server.closeConnection();
    }
}

UrlReceiver::~UrlReceiver() {
    if (thread) {
        pthread_join(thread, nullptr);
    }
}




