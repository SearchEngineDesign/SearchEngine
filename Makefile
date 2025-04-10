CXX = g++
CXXFLAGS = -std=c++17 -g


ifeq ($(OS),Windows_NT)
	OPENSSL_DIR = /usr/include/openssl
	INCLUDES = -I$(OPENSSL_DIR)
	LDFLAGS = -L/usr/lib -Lindex/stemmer/utf8proc
else
	OPENSSL_DIR = /opt/homebrew/opt/openssl@3
	INCLUDES = -I$(OPENSSL_DIR)/include 
	LDFLAGS = -L$(OPENSSL_DIR)/lib -Lindex/stemmer/utf8proc
endif



all: search

search: runner.cpp parser/HtmlParser.cpp parser/HtmlTags.cpp Crawler/crawler.cpp utils/string.cpp index/index.cpp frontier/frontier.cpp utils/Utf8.cpp distrib/node.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LDFLAGS) -lssl -lcrypto -lz -o search -g


.PHONY: clean

clean:
	rm -f search 
	find ./log/chunks -size 0 -delete