CXX = g++
CXXFLAGS = -std=c++17 -g


ifeq ($(OS),Windows_NT)
	OPENSSL_DIR = /usr/include/openssl
	INCLUDES = -I$(OPENSSL_DIR)
	LDFLAGS = -L/usr/lib -L"$(shell pwd)/index/stemmer/utf8proc"
else
	OPENSSL_DIR = /opt/homebrew/opt/openssl@3
	INCLUDES = -I$(OPENSSL_DIR)/include 
	LDFLAGS = -L$(OPENSSL_DIR)/lib -L"$(shell pwd)/index/stemmer/utf8proc"
endif



all: search

search: runner.cpp parser/HtmlParser.cpp parser/HtmlTags.cpp Crawler/crawler.cpp utils/searchstring.cpp index/index.cpp frontier/frontier.cpp utils/Utf8.cpp distrib/node.cpp distrib/URLReceiver.cpp index/stemmer/stemmer.cpp 
	install_name_tool -id "@rpath/libutf8proc.3.dylib" index/stemmer/utf8proc/libutf8proc.dylib
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LDFLAGS) -lutf8proc -lssl -lcrypto -lz -o search -g
	install_name_tool -change /usr/local/lib/libutf8proc.3.dylib @rpath/libutf8proc.3.dylib search
	install_name_tool -add_rpath "@executable_path/index/stemmer/utf8proc" search


.PHONY: clean

clean:
	rm -f search 
	find ./log/chunks -size 0 -delete