#pragma once

#include "utils/vector.h"
#include "frontier/BloomFilter.h"
#include "frontier/frontier.h"
#include "threading/ThreadPool.h"
#include "utils/ThreadSafeQueue.h"
#include "index/index.h"
// #include 

#include <optional>


struct crawlerResults {
    ParsedUrl url;
    vector<char> buffer;
    size_t pageSize;
    
    crawlerResults()=default;
    
    crawlerResults(const ParsedUrl& u, const char * v, size_t p) 
    : url(u), pageSize(p) {
        buffer.reserve(p);
        for (int i = 0; i < p; ++i)
            buffer.push_back(v[i]);
    }
    
};


//  class for abstracting distributed node
class Node {

    private:

    static constexpr float ERROR_RATE = 0.0001; // 0.01% error rate for bloom filter
    static constexpr int NUM_OBJECTS = 1000000; // estimated number of objects for bloom filter

    static constexpr int NUM_CRAWL_THREADS = 30;
    static constexpr int NUM_PARSER_THREADS = 30;

    unsigned int id;
    unsigned int numNodes;


    ThreadSafeFrontier frontier;
    ThreadSafeQueue<crawlerResults> crawlResultsQueue;
    IndexWriteHandler indexHandler;
    
    ThreadPool crawlPool;
    ThreadPool parsePool;

    vector<std::optional<Bloomfilter>> otherBloomFilters;

    Crawler alpacino; 

    public:
    Node()=default;


    Node(const unsigned int id, const unsigned int numNodes);


    void start(const string& seedlistPath) {
        frontier.buildFrontier(seedlistPath.c_str());
    }

    void shutdown(bool writeFrontier);


};