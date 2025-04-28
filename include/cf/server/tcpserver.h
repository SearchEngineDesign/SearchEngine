#pragma once

#include <string>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>


class TCPServer {

    // --- networking variables
    
    std::string ip;
    uint16_t port;


    int server_fd;
    sockaddr_in address;
    socklen_t addrlen;

    int new_socket;


    public: 
        TCPServer();

        ~TCPServer();

        void createServer(const std::string& ip, uint16_t port);

        int acceptConnection();

        void closeConnection(); 

        void closeServer();
};