#include <sys/socket.h>
#include <thread>
#include <condition_variable>
#include <iostream>
#include "crawler/crawler.h"
#include "index/index.h"

char buffer[2000000];
size_t pageSize = 0;
Crawler crawler;
UrlFrontier frontier;

Index *in;

void printBuffer() {
   for (int i = 0; i < pageSize; i++) {
      std::cout << buffer[i];
   }
}

void crawlLoop() {
   while(!frontier.empty()) {
      ParsedUrl cur = frontier.getNextUrl();
      if (crawler.crawl(cur, buffer, pageSize) == 0) {
         HtmlParser parser( buffer, pageSize );
         in->addDocument(parser);
         for (auto i : parser.links)
            frontier.addNewUrl(i.URL);
      }
   }
}

void parseLoop() {
   
}

int main(int argc, char* argv[]) {
   if (argc != 3) {
      perror("Usage: ./LinuxGetSsl [url] [index file name]");
      return 1;
   }
   frontier.addNewUrl(argv[1]);
   //in = IndexInterface(argv[2]).getIndex();
   in = new Index;
   
   std::thread t1Crawler(crawlLoop);
   std::thread t2Parser(parseLoop);

   t1Crawler.join();
   t2Parser.join();

   return 0;
}