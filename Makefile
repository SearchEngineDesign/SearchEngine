CXX = g++
CXXFLAGS = -std=c++17 -g -Iinclude -I/opt/homebrew/opt/openssl@3/include

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

SRC_FILES := $(shell find ./ ! -name "rank.cpp" ! -name "test.cpp" ! -name "constructDataset.cpp" ! -name "indexserver.cpp" ! -name "server.cpp" ! -name "testQueryCompiler.cpp" -name "*.cpp")

all: search

search: $(SRC_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LDFLAGS) $(RPATH_FLAG) -lutf8proc -lssl -lcrypto -lz -o search -g
ifeq ($(UNAME_S),Darwin)

endif

.PHONY: clean

clean:
	rm -f search 
	find ./log/chunks -size 0 -delete