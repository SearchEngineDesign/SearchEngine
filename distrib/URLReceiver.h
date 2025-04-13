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
    int port;
    int id;
    ThreadSafeFrontier * frontierPtr;

    
    pthread_t thread;
    std::atomic<bool> listenFlag;


    // --- networking variables

    int server_fd;
    sockaddr_in address;
    socklen_t addrlen;


    string parseUrls(char * buffer);


    static void * listenerEntry(void * arg) {
        auto receiver = (UrlReceiver *) arg;
        receiver->listener();
        return nullptr;
    }

    void listener();

    void createServer();


    public: 

        UrlReceiver() = default;

        UrlReceiver( const int id, const int port, ThreadSafeFrontier* frontierPtr);

        ~UrlReceiver();

        void stopListening();


        // Add other methods as needed

};