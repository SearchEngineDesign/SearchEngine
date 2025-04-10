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

static const float ERROR_RATE = 0.0001; // 0.01% error rate for bloom filter
static const int NUM_OBJECTS = 1000000; // estimated number of objects for bloom filter

static const int NUM_CRAWL_THREADS = 30;
static const int NUM_PARSER_THREADS = 30;

static Crawler alpacino; // global instance of Crawler


void parseFunc(void *arg);


volatile sig_atomic_t keep_running = 1;

// Signal handler function for SIGINT (Ctrl+C)
void handle_signal(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nInterrupt received. Shutting down gracefully..." << std::endl;
        keep_running = 0;  // Set the flag to stop the program
    }
}

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

ThreadPool crawlPool(NUM_CRAWL_THREADS);
ThreadPool parsePool(NUM_PARSER_THREADS);


void shutdown(bool writeFrontier = false) {
    crawlPool.shutdown();
    parsePool.shutdown();
    if (writeFrontier)  
        frontier.writeFrontier(1); // ? idk how to use this api
    std::cout << "Shutdown complete." << std::endl;
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
            // std::cerr << "Connection error: " << e.what() << std::endl;
            (void) e;
        }
        
        frontier.blacklist(robots.urlName);
    }
}

void crawlUrl(void *arg) {

    while (keep_running) {
        ParsedUrl url = ParsedUrl(frontier.getNextURLorWait());
    
        //crawlRobots(url.makeRobots(), url.Service + string("://") + url.Host);
    
        auto buffer = std::make_unique<char[]>(BUFFER_SIZE);
    
        size_t pageSize = 0;
    
       
    
        try {
            alpacino.crawl(url, buffer.get(), pageSize);
            crawlerResults cResult(url, buffer.get(), pageSize);
            crawlResultsQueue.put(cResult);
            
            std::cout << "Crawled: " << url.urlName << std::endl;

        } catch (const std::runtime_error &e) {
            // std::cerr << "Connection error: " << e.what() << std::endl;
            (void) e;
        }
        
    }
    

}

void parseFunc(void *arg) {


    while (keep_running) {
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
    
    // setup sigpipe handler
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, nullptr);


    for (size_t i = 0; i < NUM_CRAWL_THREADS; i++)
    {
        crawlPool.submit(crawlUrl, (void*) nullptr);
    }    
    

    for (size_t i = 0; i < NUM_PARSER_THREADS; i++)
    {
        parsePool.submit(parseFunc, (void*) nullptr);
    }    


    return 0;
}
