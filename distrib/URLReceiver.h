#pragma once

#include <atomic>
#include <cstddef>
#include "../frontier/frontier.h"
#include "pthread.h"

#include <unistd.h>


#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>



class UrlReceiver {

    private:
    int port;
    int id;
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

        UrlReceiver(ThreadSafeFrontier *frontier, const int id, const int port = 8080);

        ~UrlReceiver();

        void stopListening();


        // Add other methods as needed

};