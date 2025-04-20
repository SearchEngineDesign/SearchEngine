#include "node.h"


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
        frontier.startReturningEmpty();
        parseResultsQueue.stop();
        crawlResultsQueue.stop();
        if (writeFrontier)  
            frontier.writeFrontier(); 
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

        crawlRobots(url.makeRobots(), url.Service + string("://") + url.Host, alpacino);
    
        auto buffer = std::make_unique<char[]>(BUFFER_SIZE);
    
        size_t pageSize = 0;
    
        try {
            alpacino.crawl(url, buffer.get(), pageSize);
            crawlerResults cResult(url, buffer.get(), pageSize);
            crawlResultsQueue.put(cResult, true);
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

        frontier.insert(parser->links);
        parseResultsQueue.put(std::move(parser), true);
    }

}

void Node::index() {

    IndexWriteHandler index(CHUNK_DIR);

     while (keepRunning) {
        auto pResult = parseResultsQueue.get();
        if (pResult->base.size() != 0) {
            //std::cerr << "Indexed: " << pResult->base << std::endl;
            index.addDocument(*pResult);
        }
    }
}