#include "node.h"



void Node::handle_signal(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nInterrupt received. Shutting down gracefully..." << std::endl;
        keepRunning= false;  // Set the flag to stop the program
        shutdown(true); 
    }
}

Node::Node(const unsigned int id, const unsigned int numNodes): id(id), numNodes(numNodes), keepRunning(true),
    frontier(NUM_OBJECTS, ERROR_RATE),
    indexHandler("./log/chunks"),
    tPool(NUM_CRAWL_THREADS + NUM_PARSER_THREADS),
    alpacino(),
    urlForwarder(numNodes, id, NUM_OBJECTS, ERROR_RATE),
    crawlResultsQueue()
{

}


void Node::start(const string& seedlistPath) {
    std::cout << "Node " << id << " started." << std::endl;

    frontier.buildFrontier(seedlistPath.c_str());

    
    for (size_t i = 0; i < NUM_CRAWL_THREADS; i++)
    {
        tPool.submit(crawlEntry, (void*) this);
    }

    for (size_t i = 0; i < NUM_PARSER_THREADS; i++)
    {
        tPool.submit(parseEntry, (void*) this);
    }
    


}

void Node::shutdown(bool writeFrontier = false) {
    // ! This causes a segfault on shutdown
    // if (writeFrontier)  
        // frontier.writeFrontier(1); 
    std::cout << "Shutdown complete." << std::endl;
}



void Node::crawl() {

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


void Node::crawlRobots(const ParsedUrl& robots, const string& base) {
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



void Node::indexWrite(HtmlParser &parser) {
    switch (indexHandler.addDocument(parser)) {
        case -1:
            // whole frontier write
            std::cout << "Writing frontier and bloom filter out to file." << std::endl;
            frontier.writeFrontier(1);
            shutdown();
            break;
        case 1:
            // smini frontier write -- int 5 denotes the random chance of writing a url to the file
            std::cout << "Writing truncated frontier out to file" << std::endl;
            frontier.writeFrontier(5);
            break;
        default:
            break;
    }
}



void Node::parse() {

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