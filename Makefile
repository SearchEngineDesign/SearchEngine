CXX = g++
CXXFLAGS = -std=c++11 -g

OPENSSL_DIR = /opt/homebrew/opt/openssl@3
INCLUDES = -I$(OPENSSL_DIR)/include
LDFLAGS = -L$(OPENSSL_DIR)/lib

all: crawlParse #LinuxGetUrl

crawlParse: main.cpp parser/HtmlParser.cpp parser/HtmlTags.cpp crawler/crawler.cpp include/string.cpp indexParser/index.cpp 
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LDFLAGS) -lssl -lcrypto -lz -o crawlParse -g


.PHONY: clean

clean:
	rm -f crawlParse 
