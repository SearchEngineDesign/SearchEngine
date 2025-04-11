
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

    instance->start("./log/frontier/initlist");
    

}