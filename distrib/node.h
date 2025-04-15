#pragma once

#include "../utils/vector.h"
#include "../frontier/BloomFilter.h"
#include "../frontier/frontier.h"
#include "../threading/ThreadPool.h"
#include "../utils/ThreadSafeQueue.h"
#include "../index/index.h"
#include "../Crawler/crawler.h"

#include <optional>
#include <atomic>

#include <csignal>

#include "URLForwarder.h"

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
    
    std::atomic<bool> keepRunning;
    
    ThreadSafeFrontier frontier;
    IndexWriteHandler indexHandler;
    ThreadPool tPool;
    Crawler alpacino; 
    UrlForwarder urlForwarder;
    
    
    
    ThreadSafeQueue<crawlerResults> crawlResultsQueue;

    void crawl();

    void parse();

    void crawlRobots(const ParsedUrl& robots, const string& base);


    
    public:
    Node()=default;
    
    
    Node(const unsigned int id, const unsigned int numNodes);
    

    ~Node() {
        tPool.shutdown();
    }
    
    void start(const string& seedlistPath);

    void shutdown(bool writeFrontier);
    
    
    static void crawlEntry(void *arg) {
        auto node = static_cast<Node*>(arg);
        node->crawl();
    }
    
    static void parseEntry(void *arg) {
        auto node = static_cast<Node*>(arg);
        node->parse();
    }
    
    void indexWrite(HtmlParser &parser);


    void handle_signal(int signal);

};