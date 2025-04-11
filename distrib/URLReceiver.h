#pragma once

#include <atomic>
#include <cstddef>
#include "frontier.h"
#include "pthread.h"

#include <unistd.h>



class UrlReceiver {

    private:
    ThreadSafeFrontier *frontierPtr;
    int port;
    int id;
    pthread_t thread;
    std::atomic<bool> listenFlag;

    // --- networking variables

    int server_fd;
    sockaddr_in address;
    socklen_t addrlen;


    string UrlReceiver::parseUrls(char * buffer);


    static void * listenerEntry(void * arg) {
        auto receiver = (UrlReceiver *) arg;
        receiver->listener();
        return nullptr;
    }

    void listener();

    void createServer();


    public: 


        UrlReceiver(ThreadSafeFrontier *frontier, const int port = 8080, const int id);
        
        ~UrlReceiver();

        void stopListening();


        // Add other methods as needed

};