#include "string.h"
#include <ostream>
#include <istream>

std::ostream &operator<<( std::ostream &os, const string &s ){
   for (int i = 0; i < s.size(); i++)
      os << s[i];
   return os;
}

std::istream &operator>>( std::istream &is, string &s ) {
   char c;
   s = string();
   // Skip leading whitespace
   while (is.get(c) && isspace(c))
      continue;
   if (is) 
   {
   is.putback(c);
   // Read until whitespace or EOF
   while ( is.get(c) && !isspace(c) )
      s.push_back(c);
   }
   return is;
}

