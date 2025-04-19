#include "URLReceiver.h"


#include "../utils/searchstring.h"
#include "../frontier/frontier.h"

string UrlReceiver::parseUrls(char * buffer) {

    // delimit by new line

    auto urls = string(buffer);
    std::cout << "Receiving urls" << std::endl;  


    string substring = urls.substr(0);
    int pos = substring.find("\n");

    while (pos != -1) {
        const string& url = substring.substr(0, pos);    

        // add url to frontier
        
        if (frontierPtr && url.size() > 0) {
            frontierPtr->insertWithoutForward(url);
        }

        substring = substring.substr(pos + 1);
        pos = substring.find("\n");
    }
    

    return substring;
}

void UrlReceiver::stopListening() {
    listenFlag = false;
}

UrlReceiver::UrlReceiver( const int id_in, const uint16_t port_in, ThreadSafeFrontier* frontierPtr) :  frontierPtr(frontierPtr) {
    listenFlag = true;
    
    this->id = id_in;
    this->port = port_in + id_in;

    std::cout << "init url receiver " << this->id << " port "<< this->port <<std::endl;

    // start listener
    // (&thread, nullptr, listenerEntry, this);
}

void UrlReceiver::listener() {
    

    try {
        createServer();
        std::cout << "URLReceiver started listening on port: " << port << std::endl; 
    } catch (const std::exception &e) {
        std::cerr << "Error creating server: " << e.what() << std::endl;
        throw;
    }

    
    while (listenFlag) {
        int new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);


        if (new_socket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            sleep(1);
            continue;  // Retry accepting connections
        }


       vector<char> buffer;
       buffer.resize(1024, '\0');
       ssize_t total_received = 0;

        while (true) {
            ssize_t received = read(new_socket, buffer.data() + total_received, CHUNK_SIZE);
            if (received < 0) {
                std::cerr << "Error reading data" << std::endl;
                break;  // Exit on error
            } else if (received == 0) {
                break;  // Connection closed by the client
            }

            total_received += received;
            if (total_received >= buffer.size() - CHUNK_SIZE)
                buffer.resize(buffer.size() * 2, '\0');
        }
        parseUrls(buffer.data());
        

        // Close the connection after processing
        close(new_socket);



    }
}

UrlReceiver::~UrlReceiver() {

    if (thread) {
        pthread_join(thread, nullptr);
    }
    close(server_fd);

}

void UrlReceiver::createServer() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        throw std::runtime_error("Socket creation failed");
        // exit(EXIT_FAILURE);
    }

    // Set up address structure
    address.sin_family = AF_INET;
    address.sin_port = htons(this->port);  // Use the provided port
    

    std::string ipEnv = std::string("NODE_IP") + std::to_string(id);

    const char* ip = std::getenv(ipEnv.c_str());
    std::cout << "IP FOR CREATE SERVER IS " << ip << std::endl;

    address.sin_addr.s_addr = inet_addr(ip);
    
    int enable = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        // Handle error
    }
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        throw std::runtime_error("Socket binding failed");
        // exit(EXIT_FAILURE);
    }



    if (listen(server_fd, 3) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        throw std::runtime_error("Listening on socket failed");
    }

    addrlen = sizeof(address);


    std::cout << "URLRECEIVER " << ip << ":" << port << std::endl;
}




