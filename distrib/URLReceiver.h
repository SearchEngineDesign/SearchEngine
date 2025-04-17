#pragma once

#include <atomic>
#include <cstddef>
#include "pthread.h"
#include <iostream>

#include <unistd.h>


#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../utils/string.h"

class ThreadSafeFrontier;

class UrlReceiver {

    private:

    static const uint32_t CHUNK_SIZE = 1024;

    size_t id;
    uint16_t port;
    ThreadSafeFrontier * frontierPtr;

    
    pthread_t thread;
    std::atomic<bool> listenFlag;


    // --- networking variables

    int server_fd;
    sockaddr_in address;
    socklen_t addrlen;


    string parseUrls(char * buffer);


    
    void listener();
    
    void createServer();
    

    public: 
    
    UrlReceiver() = default;
    
    UrlReceiver( const int id, const uint16_t port, ThreadSafeFrontier* frontierPtr);
    
    ~UrlReceiver();
    
    void stopListening();
    
    static void listenerEntry(void * arg) {
        auto receiver = (UrlReceiver *) arg;
        receiver->listener();
    }
    
};