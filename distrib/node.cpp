

#include "node.h"

#include <ranges>


Node::Node(const unsigned int id, const unsigned int numNodes): id(id), numNodes(numNodes) {
    ThreadSafeFrontier frontier(NUM_OBJECTS, ERROR_RATE);
    IndexWriteHandler indexHandler("./log/chunks");
    
    ThreadPool crawlPool(NUM_CRAWL_THREADS);
    ThreadPool parsePool(NUM_PARSER_THREADS);


    otherBloomFilters.reserve(numNodes); 
    
    for (size_t i = 0; i < numNodes; i++) {
        if (i == id) {
            otherBloomFilters[i] = std::nullopt;
        } else {
            otherBloomFilters[i] = std::make_optional<Bloomfilter>(NUM_OBJECTS, ERROR_RATE);
        }
    }
}

void Node::shutdown(bool writeFrontier) {
    crawlPool.shutdown();
    parsePool.shutdown();
    if (writeFrontier)  
        frontier.writeFrontier(1); 
    std::cout << "Shutdown complete." << std::endl;
}
