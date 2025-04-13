
#include "distrib/node.h"




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


    Node node(0, 1);
     
    instance = &node;

    string seedlist = "./log/frontier/initlist";
    string bf = "./log/frontier/bloomfilter.bin";
    if (argc > 2)
        seedlist = argv[1];
    if (argc == 3)
        bf = argv[2];
    if (argc > 4)
        std::cerr << "Too many arguments. Format: ./search [path to seedlist] [path to bloomfilter]" << std::endl;

    instance->start(seedlist.c_str(), bf.c_str());
    

}