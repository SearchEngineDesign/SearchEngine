#pragma once

#include "../utils/vector.h"
#include "../frontier/BloomFilter.h"
#include <optional>
#include "../utils/crypto.h"

#include <cassert>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstdlib>
#include <cstring>

class UrlForwarder {
    private:

        static constexpr int BATCH_SIZE = 100;

        size_t numNodes;
        size_t selfId;
        // size_t NUM_OBJECTS;
        // double ERROR_RATE;
        
        vector<string> ips;

        vector<std::optional<Bloomfilter>> otherBloomFilters;
        vector<vector<string>> urlQueues;
        Crypto crypto;

        void queueSend(const string& url, const size_t id) {
            // send url to node id
            assert (id < numNodes);
            assert (id != selfId);

            auto& urlQueue = urlQueues[id];

            urlQueue.push_back(url);

            if (urlQueue.size() >= BATCH_SIZE) {
                // send urlQueue to node id
                // clear urlQueue
                sendBatch(id);
                urlQueues[id].clear();
            }

        }


        void sendBatch(const size_t id) {
            const string& ip = ips[id];
            const uint16_t port = 8080 + id;

            auto& urls = urlQueues[id];
            

            int sockfd = socket(AF_INET, SOCK_STREAM, 0);


            sockaddr_in serv_addr = {};
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);
        
            if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                // Handle error
                close(sockfd);
                return;
            }


             // Serialize vector<string> into a flat buffer
            string payload;
            for (const auto& url : urls) {
                payload.append(url);
                payload.push_back('\n');
            }

             if (send(sockfd, payload.c_str(), payload.size(), 0) != 0) {
                // Handle error
                std::cerr << "Error sending data to node " << id << std::endl;
             } else {
                urls.clear();
             }

            close(sockfd);

        }


    public:

    UrlForwarder() = default;

    UrlForwarder(size_t numNodes, size_t id) : numNodes(numNodes), selfId(id) {
        
        // init bloom filters
        otherBloomFilters.reserve(numNodes); 
        for (size_t i = 0; i < numNodes; i++) {
            if (i == id) {
                // otherBloomFilters[i] = std::nullopt;
                otherBloomFilters.emplace_back(std::nullopt);
            } else {
                // otherBloomFilters[i].emplace(NUM_OBJECTS, ERROR_RATE);
                otherBloomFilters.emplace_back(Bloomfilter(true));
            }
        }

        
        // init ips
        ips.reserve(numNodes);
        for (size_t i = 0; i < numNodes; i++)
        {
            // MAN I LOVE REWRITING THE STRING CLASS
            const string envVar = string("NODE_IP") + string(std::to_string(i).c_str());
            // ips[i] = std::getenv(envVar.c_str());
        
            ips.emplace_back(std::getenv(envVar.c_str()));
        }
        

    }

    // TODO: add dist from seedlist
    int addUrl(const string& url) {
        const unsigned int urlOwner = crypto.hashMod(url, numNodes);

        if (not otherBloomFilters[urlOwner].has_value()) {
            assert(urlOwner == selfId);
            return urlOwner;
        }

        
        auto& bloomFilter = otherBloomFilters[urlOwner].value();

        const auto otherHasValue = bloomFilter.contains(url);

        if (otherHasValue) {
            return -1;
        }
        

        bloomFilter.insert(url);
        queueSend(url, urlOwner);
        
        return urlOwner;

    }
};





