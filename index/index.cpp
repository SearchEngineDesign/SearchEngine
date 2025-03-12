//index.cpp

#include "index.h"

void Index::addDocument(HtmlParser &parser) {
   Tuple<string, PostingList> *seek;
   string concat;
   for (auto i : parser.bodyWords) {
      seek = dict.Find(i.first, PostingList(i.first, Token::Body));
      seek->value.appendBodyDelta(i.second + WordsInIndex, 0);
   }
   for (auto i : parser.headWords) {
      seek = dict.Find(i.first, PostingList(i.first, Token::Body));
      seek->value.appendBodyDelta(i.second + WordsInIndex, 3);
   }
   for (auto i : parser.boldWords) {
      seek = dict.Find(i.first, PostingList(i.first, Token::Body));
      seek->value.appendBodyDelta(i.second + WordsInIndex, 2);
   }
   for (auto i : parser.italicWords) {
      seek = dict.Find(i.first, PostingList(i.first, Token::Body));
      seek->value.appendBodyDelta(i.second + WordsInIndex, 1);
   }
   for (auto i : parser.titleWords) {
      concat = titleMarker + i.first;
      seek = dict.Find(concat, PostingList(concat, Token::Title));
      seek->value.appendTitleDelta(i.second + WordsInIndex);
   }

   for (auto i : parser.links) {
      //TODO: implement a better way to index anchor text
      bool first = true;
      for (auto j : i.anchorText) {
         if (first) {
            //places URL delta at beginning of corresponding anchor text
            concat = urlMarker + i.URL;
            seek = dict.Find(concat, PostingList(concat, Token::URL));
            seek->value.appendTitleDelta(j.second + WordsInIndex); 
            first = false;
         }
         concat = anchorMarker + j.first;
         seek = dict.Find(concat, PostingList(concat, Token::Anchor));
         seek->value.appendTitleDelta(j.second + WordsInIndex);
      }     
   }
   seek = dict.Find(eodMarker, PostingList(eodMarker, Token::EoD));
   seek->value.appendEODDelta(parser.count + WordsInIndex, DocumentsInIndex);
   
   WordsInIndex += parser.count;
   DocumentsInIndex += 1;
   documents.push_back(parser.base);
}

//for utillity
uint8_t bitsNeeded(const size_t n) {
    if (n == 0) {
        return 1; 
    }
    return std::max(1, static_cast<int>(std::ceil(std::log2(n + 1))));
}

char *formatUtf8(const size_t &delta) {
   const uint8_t boundary = bitsNeeded(delta);
   size_t bytes = 0;
   if (boundary < 7)
      bytes = 1;
   else if (boundary < 12)
      bytes = 2;
   else if (boundary < 17)
      bytes = 3;
   else if (boundary < 22)
      bytes = 4;
   else if (boundary < 27)
      bytes = 5;
   else if (boundary < 32)
      bytes = 6;
   else if (boundary < 37)
      bytes = 7;

   char* bitset = new char[bytes];
   uint8_t bitsetIndex = 0, initDelta = 0, deltaIndex = 0, index = bytes;
   
   while(deltaIndex < boundary) { 
      if (bitsetIndex % 8 == 0)
         index--;
      initDelta = deltaIndex + 6;
      for (; deltaIndex < initDelta && deltaIndex < boundary; deltaIndex++) {
         if ((delta >> deltaIndex) & 1)
            bitset[index] |= 1 << bitsetIndex;
         bitsetIndex++;
      }
      if (deltaIndex < boundary) {
         bitset[index] |= 0 << bitsetIndex;
         bitset[index] |= 1 << (bitsetIndex + 1);
         bitsetIndex += 2;
      }
      bitsetIndex = bitsetIndex % 8;
   }
   for (int i = 7; i > 7 - bytes; i--)
      bitset[0] |= 1 << i;

   return bitset;
}

void PostingList::appendTitleDelta(const size_t delta) {
   list.emplace_back(formatUtf8(delta));  
}

void PostingList::appendBodyDelta(size_t delta, uint8_t style) {
   delta = delta << 2;
   delta += style;
   list.emplace_back(formatUtf8(delta)); 
}

void PostingList::appendEODDelta(size_t delta, size_t docIndex) {
   //TODO: tweak how we process these
   delta = delta << sizeof(docIndex);
   delta += docIndex;
   list.emplace_back(formatUtf8(delta));  
}