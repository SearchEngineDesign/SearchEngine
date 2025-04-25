#include "tcpserver.h"
#include <iostream>


TCPServer::TCPServer() {
    server_fd = -1;
    new_socket = -1;
}

void TCPServer::createServer(const std::string& ip, uint16_t port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        throw std::runtime_error("Socket creation failed");
        // exit(EXIT_FAILURE);
    }

    // Set up address structure
    address.sin_family = AF_INET;
    address.sin_port = htons(this->port);  // Use the provided port
    address.sin_addr.s_addr = inet_addr(this->ip.c_str());
    
    
    int enable = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        // Handle error
    }
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
    }



    if (listen(server_fd, 3) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        throw std::runtime_error("Listening on socket failed");
    }

    addrlen = sizeof(address);
}


int TCPServer::acceptConnection() {
    this->new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
        throw std::runtime_error("Accepting connection failed");
    }

    return new_socket;
}

void TCPServer::closeServer() {
    closeConnection();
    
    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }
}

TCPServer::~TCPServer() {
    closeServer();
}

void TCPServer::closeConnection() {
    if (new_socket != -1) {
        close(new_socket);
        new_socket = -1;
    }
}
