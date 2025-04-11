#include "index/index.h"
#include "utils/string.h"
#include <filesystem>
#include <iostream>

int main(int argc, char * argv[]) {
    bool verbose = false;
    // provide the name of a folder
    if (argc < 2 || argc > 4)
        return 1;
    else if (argc  == 3)
        if (strcmp(argv[2], "-v") == 0 || strcmp(argv[2], "--verbose") == 0)
            verbose = true;
    //for (const auto& entry : std::filesystem::directory_iterator(argv[1])) {
        //std::cout << entry.path().filename().c_str();
        try {
            IndexReadHandler::testReader(argv[1], verbose);
        } catch(const std::runtime_error& error) {
            std::cout << "Error reading " << string(argv[1]) << ": " << string(error.what()) << std::endl;
        }
    //}

    return 0;
}