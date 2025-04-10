#include "Crawler/crawler.h"
#include "parser/HtmlParser.h"
#include <cstddef>
#include <pthread.h>
#include "utils/vector.h"
#include "frontier/frontier.h"
#include "index/index.h"

#include "utils/string.h"
#include "utils/ThreadSafeQueue.h"

#include "frontier/frontier.h"

#include "threading/ThreadPool.h"

#include <csignal>

#include <chrono>
#include <atomic>

static const float ERROR_RATE = 0.0001; // 0.01% error rate for bloom filter
static const int NUM_OBJECTS = 1000000; // estimated number of objects for bloom filter

static const int NUM_CRAWL_THREADS = 10;
static const int NUM_PARSER_THREADS = 10;

static Crawler alpacino; // global instance of Crawler


void parseFunc(void *arg);



std::atomic_bool keepRunning(true);




struct crawlerResults {
    ParsedUrl url;
    vector<char> buffer;
    size_t pageSize;
    
    crawlerResults() : url(""), pageSize(0) {}
    
    crawlerResults(const ParsedUrl& u, const char * v, size_t p) 
    : url(u), pageSize(p) {
        buffer.reserve(p);
        for (int i = 0; i < p; ++i)
            buffer.push_back(v[i]);
    }
    
};

ThreadSafeFrontier frontier(NUM_OBJECTS, ERROR_RATE);
ThreadSafeQueue<crawlerResults> crawlResultsQueue;
IndexWriteHandler indexHandler("./log/chunks");


static ThreadPool tPool(NUM_CRAWL_THREADS + NUM_PARSER_THREADS);


void shutdown(bool writeFrontier = false) {
    // crawlPool.shutdown();
    // parsePool.shutdown();
    // if (writeFrontier)  
        // frontier.writeFrontier(1); // this is causing segfault, filedescriptor is wrong 


    sleep(5);

    // insert dummy values to the queue to wake up the threads
    frontier.emptyFrontier();

    frontier.startReturningEmpty();

    crawlResultsQueue.emptyQueue();
    for (size_t i = 0; i < NUM_PARSER_THREADS; i++) {
        crawlResultsQueue.put(crawlerResults());
    }
    
    std::cout << "Shutdown complete." << std::endl;

}


void handle_signal(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nInterrupt received. Shutting down gracefully..." << std::endl;
        keepRunning= false;  // Set the flag to stop the program
        shutdown(true); 
    }

}

void indexWrite(HtmlParser &parser) {
    switch (indexHandler.addDocument(parser)) {
        case -1:
            // whole frontier write
            std::cout << "Writing frontier and bloom filter out to file." << std::endl;
            frontier.writeFrontier(1);
            shutdown();
            break;
        case 1:
            // mini frontier write -- int 5 denotes the random chance of writing a url to the file
            std::cout << "Writing truncated frontier out to file" << std::endl;
            frontier.writeFrontier(5);
            break;
        default:
            break;
    }
}

void crawlRobots(const ParsedUrl& robots, const string& base) {
    if (!frontier.contains(robots.urlName)) {

        // use unique ptr
        auto buffer = std::make_unique<char[]>(BUFFER_SIZE);
        

        size_t pageSize = 0;
        try {
            alpacino.crawl(robots, buffer.get(), pageSize);
            const char * c = buffer.get();
            HtmlParser parser(c, pageSize, base);
            crawlerResults cResult(robots, buffer.get(), pageSize);
            for (const auto &goodlink : parser.bodyWords) {
                frontier.insert(goodlink);
            }
            for (const auto &badlink : parser.headWords) {
                frontier.blacklist(badlink);
            }
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
        
        frontier.blacklist(robots.urlName);
    }
}

void crawlUrl(void *arg) {

    while (keepRunning) {
        ParsedUrl url = ParsedUrl(frontier.getNextURLorWait());
    

        std::cout << "Started Crawling: " << url.urlName << std::endl;

        if (url.urlName.empty()){
            std::cout << "Crawl func exiting because URL was empty" << std::endl;
            std::cout << "This behaviour should only happen when the program is shutting down" << std::endl;
            assert(keepRunning == false);
            break;
        }

        crawlRobots(url.makeRobots(), url.Service + string("://") + url.Host);
    
        auto buffer = std::make_unique<char[]>(BUFFER_SIZE);
    
        size_t pageSize = 0;
    
       
    
        try {
            alpacino.crawl(url, buffer.get(), pageSize);
            crawlerResults cResult(url, buffer.get(), pageSize);
            crawlResultsQueue.put(cResult);
            
            std::cout << "Crawled: " << url.urlName << std::endl;

        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
        
    }

    

}

void parseFunc(void *arg) {


    while (keepRunning) {
        crawlerResults cResult = crawlResultsQueue.get();
    
        HtmlParser parser(cResult.buffer.data(), cResult.pageSize);

        std::cout << "Parsed: " << cResult.url.urlName << std::endl;

        for (const auto &link : parser.links) {
            frontier.insert(link.URL);
        }
        
        if (parser.base.size() != 0) {
            std::cout << "Indexed: " << cResult.url.urlName << std::endl;
            indexWrite(parser);
        }
    }

}

int main(int argc, char * argv[]) {
    if (argc == 2) {
        std::cout << "Building frontier with specified seedlist." << std::endl;
        if (frontier.buildFrontier(argv[1]) == 1) {
            std::cerr << "Expected input: ./search [path to seedlist]" << std::endl;
            shutdown();
            return 1;
        }
    } else if (argc == 3) {
        std::cout << "Building frontier with specified seedlist." << std::endl;
        if (frontier.buildFrontier(argv[1]) == 1) {
            std::cerr << "Expected input: ./search [path to seedlist]" << std::endl;
            shutdown();
            return 1;
        }
        std::cout << "Building bloom filter with specified file." << std::endl;
        if (frontier.buildBloomFilter(argv[2]) == 1) {
            std::cerr << "Expected input: ./search [path to seedlist] [path to bloomfilter]" << std::endl;
            shutdown();
            return 1;
        }
    } else if (argc > 3) {
        std::cerr << "Expected input: ./search [seedlist]]" << std::endl;
        shutdown();
        return 1;
    } else {   
        std::cout << "Building frontier with default seedlist." << std::endl;
        if (frontier.buildFrontier("./log/frontier/initlist") == 1) {
            shutdown();
            return 1;
        }
    }


    signal(SIGINT, handle_signal); // Register the signal handler for SIGINT



    for (size_t i = 0; i < NUM_CRAWL_THREADS; i++)
    {
        tPool.submit(crawlUrl, (void*) nullptr);
    }    
    

    for (size_t i = 0; i < NUM_PARSER_THREADS; i++)
    {
        tPool.submit(parseFunc, (void*) nullptr);
    }    


    return 0;
}
