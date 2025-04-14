#pragma once

#include "../utils/vector.h"
#include "../frontier/BloomFilter.h"
#include "../frontier/frontier.h"
#include "../threading/ThreadPool.h"
#include "../utils/ThreadSafeQueue.h"
#include "../index/index.h"
#include "../Crawler/crawler.h"

#include <chrono>
#include <fstream>
#define timeNow() std::chrono::high_resolution_clock::now()
#define duration(a) std::chrono::duration_cast<std::chrono::nanoseconds>(a).count()
typedef std::chrono::high_resolution_clock::time_point TimeVar;

#include <atomic>

#include <csignal>

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

    struct Stats {
        size_t sitescrawled = 0;
        float dur;

        TimeVar start = timeNow();
        TimeVar stop;


        void report(int sz, Node *node) {
            
            stop = timeNow();
            dur = duration(stop - start);
            start = timeNow();
            
            std::ofstream file("./log/stats/avg");
            std::streambuf* cout_sbuf = std::cout.rdbuf(); // Save cout's original buffer
            std::cout.rdbuf(file.rdbuf()); // Redirect cout to file

            sitescrawled += sz;
            std::cout << "Indexed " << sz << " documents in " << dur << " seconds." << std::endl;
            std::cout << "Average: " << float(sz / dur) << " documents / second." << std::endl;
            std::cout << sitescrawled << " documents total." << std::endl << std::endl;

            std::cout << node->crawlResultsQueue.size() << " items in Crawl Results Queue" << std::endl;
            std::cout << node->frontier.size() << " items in Frontier" << std::endl;

            std::cout.rdbuf(cout_sbuf); // Restore cout's original buffer
        }
    };

    friend class Stats;

    Stats stats;

    static constexpr float ERROR_RATE = 0.0001; // 0.01% error rate for bloom filter
    static constexpr int NUM_OBJECTS = 1000000; // estimated number of objects for bloom filter

    static constexpr int NUM_CRAWL_THREADS = 256;
    static constexpr int NUM_PARSER_THREADS = 256;

    unsigned int id;
    unsigned int numNodes;
    
    std::atomic<bool> keepRunning;

    ThreadSafeFrontier frontier;
    IndexWriteHandler indexHandler;
    ThreadSafeQueue<crawlerResults> crawlResultsQueue;
    ThreadPool tPool;

    void crawl();

    void parse();

    void crawlRobots(const ParsedUrl& robots, const string& base, Crawler &alpacino);


    
    public:
    Node()=default;
    
    
    Node(const unsigned int id, const unsigned int numNodes);
    
    
    void start(const char * seedlistPath, const char * bfPath);

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



