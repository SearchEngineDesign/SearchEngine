#include "node.h"


void Node::handle_signal(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nInterrupt received. Shutting down gracefully..." << std::endl;
        shutdown(true); 
        keepRunning = false;  // Set the flag to stop the program

        for (size_t i = 0; i < numNodes; i++) {
            if(i != id) {
                urlReceivers[i].stopListening();
            }
        }
    }
}

Node::Node(const unsigned int id, const unsigned int numNodes): id(id), numNodes(numNodes), keepRunning(true),
    frontier(numNodes, id),
    indexHandler("./log/chunks"),

    crawlResultsQueue(),
    urlReceivers(new UrlReceiver[numNodes]),
    tPool(NUM_CRAWL_THREADS + NUM_PARSER_THREADS + NUM_INDEX_THREADS + numNodes)
{
    for (size_t i = 0; i < numNodes; i++) {
        if(i == id) {
            new (&urlReceivers[i]) UrlReceiver(i, 8080, &frontier);

        } else {
            // urlReceivers.push_back(nullptr);
        }
    }
    
}

void Node::start(const char * seedlistPath, const char * bfPath) {
    std::cout << "Node " << id << " started." << std::endl;

    if (frontier.buildFrontier(seedlistPath, bfPath) == 1) {
        shutdown(false);
        return;
    }




    
    for (size_t i = 0; i < NUM_CRAWL_THREADS; i++)
    {
        tPool.submit(crawlEntry, (void*) this);
    }

    for (size_t i = 0; i < NUM_PARSER_THREADS; i++)
    {
        tPool.submit(parseEntry, (void*) this);
    }


    for (size_t i = 0; i < numNodes; i++)
    {
        if(i == id) {
            tPool.submit(urlReceivers[i].listenerEntry, (void*) &urlReceivers[i]);
        }
    }
    
    for (size_t i = 0; i < NUM_INDEX_THREADS; i++)
    {
        tPool.submit(indexEntry, (void*) this);
    }

}

void Node::shutdown(bool writeFrontier) {
    if (keepRunning) {
        if (writeFrontier)  
            frontier.writeFrontier(); 
        std::cout << "Shutdown complete." << std::endl;
    }
}



void Node::crawl() {

    Crawler alpacino;

    while (keepRunning) {
        crawlResultsQueue.wait();

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
            crawlResultsQueue.put(cResult);

            if (crawlResultsQueue.size() > 10000 or crawlResultsQueue.size() < 0) {
                std::cout << "Crawl Results Queue size: " << crawlResultsQueue.size() << std::endl;
                std::cout << "url: " << url.Host << std::endl;
                exit(1);
            }
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
    int count = indexHandler.index->DocumentsInIndex;
    switch (indexHandler.addDocument(parser)) {
        case -1:
            // whole frontier write
            std::cout << "Writing frontier and bloom filter out to file." << std::endl;
            stats.report(count, this);
            frontier.writeFrontier();
            break;
        case 1:
            stats.report(count, this);
            break;
        default:
            break;
    }
}



void Node::parse() {

    while (keepRunning) {
        crawlerResults cResult = crawlResultsQueue.get();
    
        auto parser = std::make_unique<HtmlParser>(cResult.buffer.data(), cResult.pageSize);

        for (const auto &link : parser->links) {
            frontier.insert(link.URL);
        }

        parseResultsQueue.put(std::move(parser));
    }

}

void Node::index() {
     while (keepRunning) {
        auto pResult = parseResultsQueue.get();

        
        if (pResult->base.size() != 0 && rand() % 100 == 0) {
            std::cout << "Indexed: " << pResult->base << std::endl;
            indexWrite(*pResult);
        }
    }
}