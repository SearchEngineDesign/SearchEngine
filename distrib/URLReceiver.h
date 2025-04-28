#pragma once

#include <atomic>
#include <cstddef>
#include "pthread.h"
#include <iostream>


#include <cf/searchstring.h>
#include <cf/server/tcpserver.h>

class ThreadSafeFrontier;

class UrlReceiver {

    private:

    static const uint32_t CHUNK_SIZE = 1024 * 4;

    size_t id;
    uint16_t port;
    ThreadSafeFrontier * frontierPtr;

    
    pthread_t thread;
    std::atomic<bool> listenFlag;

    TCPServer server;


    string parseUrls(const char *buffer);

    void listener();
    

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