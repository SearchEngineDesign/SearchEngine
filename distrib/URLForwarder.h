#pragma once

// #include "../utils/vector.h"

#include <cf/vec.h>
#include <cf/crypto.h>

#include "../frontier/BloomFilter.h"

#include <cassert>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstdlib>
#include <cstring>

#include <cf/threading/ThreadPool.h>

class UrlForwarder {
    private:

        static constexpr int BATCH_SIZE = 1000;

        size_t numNodes;
        size_t selfId;
        
        vector<string> ips;

        vector<Bloomfilter> bloomFilters;
        vector<vector<string>> urlQueues;
        Crypto crypto;


        
        ThreadPool tPool;

        inline void queueSend(const string& url, const size_t id) {
            // send url to node id

            assert (id < numNodes);

            if (id == selfId) {
                // do not send to self
                return;
            }


            auto& urlQueue = urlQueues[id];

            urlQueue.push_back(url);

            if (urlQueue.size() >= BATCH_SIZE) {
                // send urlQueue to node id
                // clear urlQueue

                auto *sendBatchArgs = new SendBatchArgs;
                sendBatchArgs->id = id;
                sendBatchArgs->urls = std::move(urlQueue);
                sendBatchArgs->ip = ips[id];

                tPool.submit(sendBatch, (void *) sendBatchArgs);
                
                urlQueues[id].clear();
                urlQueues[id].reserve(BATCH_SIZE);
            }

        }

        struct SendBatchArgs {
            size_t id;
            vector<string> urls;
            string ip;
        };


        static void sendBatch(void * arg) {
            auto* args = static_cast<SendBatchArgs*>(arg);
            size_t id = args->id;
            string& ip = args->ip;
            vector<string>& urls = args->urls;

            const uint16_t port = 8080;

            

            int sockfd = socket(AF_INET, SOCK_STREAM, 0);

            sockaddr_in serv_addr = {};
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);
            std::cerr << "Sending batch to node " << id << " at addr " << ip << std::endl;
            if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                // Handle error
                std::cerr << "Error connecting to node " << id << std::endl;
                close(sockfd);
                return;
            }


             // Serialize vector<string> into a flat buffer
            string payload;
            for (const auto& url : args->urls) {
                payload.append(url);
                payload.push_back('\n');
            }


            uint32_t len = htonl(payload.size());
            if (send(sockfd, &len, sizeof(len), 0) != 0) {
                std::cerr << "Error sending length to node " << id << std::endl;
                close(sockfd);
                delete args;
                return;
            }

            if (send(sockfd, payload.c_str(), payload.size(), 0) != 0) {
                // Handle error
                std::cerr << "Error sending data to node " << id << std::endl;
            } 

            delete args;
            close(sockfd);
        }


    public:

    UrlForwarder() = default;

    UrlForwarder(size_t numNodes, size_t id) : numNodes(numNodes), selfId(id) {
        

        urlQueues.resize(numNodes, vector<string>());

        // init bloom filters
        bloomFilters.reserve(numNodes); 
        for (size_t i = 0; i < numNodes; i++) {
            bloomFilters.emplace_back(true);
        }

        
        // init ips
        ips.reserve(numNodes);
        for (size_t i = 0; i < numNodes; i++)
        {
            const string envVar = string("NODE_IP") + string(std::to_string(i).c_str());
        
            ips.emplace_back(std::getenv(envVar.c_str()));
        }
        

    }

    // TODO: add dist from seedlist
    inline std::pair<int, bool> addUrl(const string& url) {
        const unsigned int urlOwner = crypto.hashMod(url, numNodes);
        auto& bloomFilter = bloomFilters[urlOwner];

        bool alreadySeen = bloomFilter.contains(url);

        if (alreadySeen == false) {
            queueSend(url, urlOwner);
        }
        
        return std::make_pair(urlOwner, alreadySeen);
    }


    inline Bloomfilter& getBloomFilter(const size_t id) {
        return bloomFilters[id];
    }
};





