#include "node.h"
#include <chrono>
#include <thread>
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds


void Node::handle_signal(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nInterrupt received. Shutting down gracefully..." << std::endl;
        shutdown(true); 
        urlReceiver->stopListening();
    }
}

Node::Node(const unsigned int id_in, const unsigned int numNodes): 
    id(id_in), 
    numNodes(numNodes), 
    keepRunning(true),
    frontier(numNodes, id_in),

    crawlResultsQueue(),
    urlReceiver(),
    tPool(NUM_CRAWL_THREADS + NUM_PARSER_THREADS + NUM_INDEX_THREADS + 1)
{
    urlReceiver = std::make_shared<UrlReceiver>(id, 8080, &frontier);
    
}

void testindex(const char * CHUNK_DIR) {
    const string tocrawl[13] = {
        "https://bandcamp.com/",
        "https://soundcloud.com/",
        "https://rateyourmusic.com/",
        "https://finance.yahoo.com/",
        "https://www.yahoo.com/",
        "https://www.google.com/",
        "https://www.bing.com/",
        "https://www.instagram.com/",
        "https://www.marxists.org/english.htm",
        "https://www.progarchives.com/",
        "https://www.britannica.com/",
        "https://www.ebay.com/",
        "https://www.amazon.com/"
    };
    IndexWriteHandler iwh(CHUNK_DIR);
    Crawler c;
    for (auto i : tocrawl) {
        auto url = ParsedUrl(i);
        auto buffer = std::make_unique<char[]>(BUFFER_SIZE);
        size_t pageSize = 0;
        c.crawl(url, buffer.get(), pageSize);
        crawlerResults cResult(url, buffer.get(), pageSize);
        auto parser = std::make_unique<HtmlParser>(cResult.buffer.data(), cResult.pageSize);
        iwh.addDocument(*parser);
    }
    string fname = iwh.getFilename();
    iwh.WriteIndex();
    auto t = iwh.index->getDict()->Find("word");
    assert(t != nullptr);
    auto t1 = iwh.index->getDict()->Find("start");
    assert(t1 != nullptr);

    IndexReadHandler ihr;
    ihr.ReadIndex(fname.c_str());
}

void Node::start(const char * seedlistPath, const char * bfPath) {
    std::cout << "Node " << id << " started." << std::endl;

    if (frontier.buildFrontier(seedlistPath, bfPath) == 1) {
        shutdown(false);
        return;
    }

    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);

    for (size_t i = 0; i < NUM_CRAWL_THREADS; i++)
    {
        tPool.submit(crawlEntry, (void*) this);
    }

    for (size_t i = 0; i < NUM_PARSER_THREADS; i++)
    {
        tPool.submit(parseEntry, (void*) this);
    }

    tPool.submit(urlReceiver->listenerEntry, (void*) urlReceiver.get());
    
    
    for (size_t i = 0; i < NUM_INDEX_THREADS; i++)
    {
        tPool.submit(indexEntry, (void*) this);
    }
}

void Node::shutdown(bool writeFrontier) {
    if (keepRunning) {
        keepRunning = false;
        std::cout << frontier.size() << " items in frontier." << std::endl;
        parseResultsQueue.stop();
        if (writeFrontier)  
            frontier.writeFrontier(); 
        frontier.startReturningEmpty();
        crawlResultsQueue.stop();
        std::cout << "Shutdown complete." << std::endl;
    }
}



void Node::crawl() {
    Crawler alpacino;

    while (keepRunning) {

        auto url = ParsedUrl(frontier.getNextURLorWait());
    
        if (url.urlName.empty()){
            std::cout << "Crawl func exiting because URL was empty" << std::endl;
            std::cout << "This behaviour should only happen when the program is shutting down" << std::endl;
            assert(keepRunning == false);
            break;
        }
        //TODO uncomment
        crawlRobots(url.makeRobots(), url.Service + string("://") + url.Host, alpacino);
    
        auto buffer = std::make_unique<char[]>(BUFFER_SIZE);
    
        size_t pageSize = 0;
    
        try {
            alpacino.crawl(url, buffer.get(), pageSize);
            crawlerResults cResult(url, buffer.get(), pageSize);
            crawlResultsQueue.put(std::move(cResult), false);
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
        
    }

}


void Node::crawlRobots(const ParsedUrl& robots, const string& base, Crawler &alpacino) {
    if (!frontier.contains(robots.urlName)) {

        // use unique ptr
        auto buffer = std::make_unique<char[]>(BUFFER_SIZE);
        

        size_t pageSize = 0;
        try {
            alpacino.crawl(robots, buffer.get(), pageSize);
            const char * c = buffer.get();
            HtmlParser parser(c, pageSize, base);
            crawlerResults cResult(robots, buffer.get(), pageSize);

            frontier.insert(parser.bodyWords);

            for (const auto &badlink : parser.headWords) {
                frontier.blacklist(badlink);
            }


        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }
        
        frontier.blacklist(robots.urlName);
    }
}


void Node::parse() {
    while (keepRunning) {
        crawlerResults cResult = crawlResultsQueue.get();
    
        auto parser = std::make_unique<HtmlParser>(cResult.buffer.data(), cResult.pageSize);
        //TODO uncomment
        frontier.insert(parser->links);
        parseResultsQueue.put(std::move(parser), false);
    }

}

void Node::index() {

    IndexWriteHandler index(CHUNK_DIR);

     while (keepRunning) {
        auto pResult = parseResultsQueue.get();
        if (!keepRunning)
            break;
        if (pResult->base.size() != 0) {
            index.addDocument(*pResult);
        }
    }
}