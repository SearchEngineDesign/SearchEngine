#pragma once
// #include "utils/string.h"
// #include <string>
#include "include/string.h"

class ParsedUrl {
    public:
        string urlName;
        string Service, Host, Port, Path;
    
        ParsedUrl(const string& url) {
            urlName = url;
    
            size_t pos = 0;
            const char *colon = ":";
            const char *slash = "/";
            int colonPos = url.find(colon);
            int slashPos = url.find(slash, colonPos + 2);
    
            if (colonPos != -1) {
                // Extract Service
                Service = url.substr(0, colonPos);
                pos = colonPos + 1;
    
                // Skip "://" if present
                if (*url.at(pos) == '/' && *url.at(pos+1) == '/') {
                    pos += 2;
                }
    
                // Extract Host
                int hostEnd = url.find(slash, pos);
                hostEnd == -1 ? hostEnd = url.length() : hostEnd = hostEnd;
                Host = url.substr(pos, hostEnd - pos);
                pos += Host.length();
    
                // Extract Port if present
                if (*url.at(pos) == ':') {
                    pos++;
                    int portEnd = url.find(slash, pos);
                    Port = url.substr(pos, portEnd - pos);
                    pos += portEnd;
                }
    
                // Extract Path
                if (pos < url.length()) {
                    Path = url.substr(pos, url.length() - pos);
                    if (Path == "/")
                        Path = "";
                }
            } else {
                Host = url;
                Path = "";
            }
        }
    
        ~ParsedUrl() {
            // No manual memory management required with string
        }
    };