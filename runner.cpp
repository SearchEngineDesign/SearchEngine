
#include "distrib/node.h"


#include <cstdlib>

static Node *instance;


void handle_signal(int signal) {
    instance->handle_signal(signal);
}


int main(int argc, char * argv[]) {

    // setup sigpipe handler
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, nullptr);


    signal(SIGINT, handle_signal); // Register the signal handler for SIGINT    


    const char * numNodesStr = std::getenv("NUM_NODES");
    if (numNodesStr == nullptr) {
        std::cerr << "NUM_NODES environment variable not set." << std::endl;
        return 1;
    }
    const char * idStr = std::getenv("NODE_ID");
    if (idStr == nullptr) {
        std::cerr << "NODE_ID environment variable not set." << std::endl;
        return 1;
    }

    const int numNodes = atoi(numNodesStr);

    for (size_t i = 0; i < numNodes; i++) {
        if (std::getenv(("NODE_IP" + std::to_string(i)).c_str()) == nullptr) {
            std::cerr << "NODE_IP environment variable is not set for node: " << i << std::endl;
            return 1;
        }
    }


    
    const int id = atoi(idStr);

    Node node(id, numNodes);
     
    instance = &node;

    string seedlist = "./log/frontier/initlist";
    string bf = "./log/frontier/bloomfilter.bin";
    if (argc > 1)
        seedlist = argv[1];
    
    if (argc == 3)
        bf = argv[2];
    if (argc > 4)
        std::cerr << "Too many arguments. Format: ./search [path to seedlist] [path to bloomfilter]" << std::endl;

    instance->start(seedlist.c_str(), bf.c_str());
    

}