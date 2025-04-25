CXX = g++
CXXFLAGS = -std=c++17 -g -Iinclude

ifeq ($(OS),Windows_NT)
	OPENSSL_DIR = /usr/include/openssl
	INCLUDES = -I$(OPENSSL_DIR)
	LDFLAGS = -L/usr/lib -L"$(shell pwd)/index/stemmer/utf8proc"
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		# macOS specific settings
		OPENSSL_DIR = /opt/homebrew/opt/openssl@3
		INCLUDES = -I$(OPENSSL_DIR)/include 
		LDFLAGS = -L$(OPENSSL_DIR)/lib -L"$(shell pwd)/index/stemmer/utf8proc"
		RPATH_FLAG = -Wl,-rpath,"@executable_path/index/stemmer/utf8proc"
	else
		# Linux specific settings
		OPENSSL_DIR = /usr/include/openssl
		INCLUDES = -I$(OPENSSL_DIR)
		LDFLAGS = -L/usr/lib -L"$(shell pwd)/index/stemmer/utf8proc"
		RPATH_FLAG = -Wl,-rpath,"$(shell pwd)/index/stemmer/utf8proc"
	endif
endif

all: search

search: runner.cpp parser/HtmlParser.cpp parser/HtmlTags.cpp Crawler/crawler.cpp utils/searchstring.cpp index/index.cpp utils/Utf8.cpp distrib/node.cpp distrib/URLReceiver.cpp index/stemmer/stemmer.cpp utils/IndexBlob.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LDFLAGS) $(RPATH_FLAG) -lutf8proc -lssl -lcrypto -lz -o search -g
ifeq ($(UNAME_S),Darwin)

endif

.PHONY: clean

clean:
	rm -f search 
	find ./log/chunks -size 0 -delete