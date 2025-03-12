#include "string.h"

std::ostream &operator<<( std::ostream &os, const string &s )
   {
      os << s.cstr();
      return os;
   }