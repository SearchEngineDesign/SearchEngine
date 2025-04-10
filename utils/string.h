// string.h
// 
// Starter file for a string template

#pragma once
#include <cstddef>   // for size_t
#include <ostream>  // for ostream
#include <unistd.h>
#include <cstring>

class string
   {
   public:  

      // Default Constructor
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Creates an empty string
      string( ) : m_size( 0 ), m_capacity( 1 ), m_data( new char[ 1 ] )
         {
            m_data = new char[1];
            *m_data = '\0';
         }

      // string Literal / C string Constructor
      // REQUIRES: cstr is a null terminated C style string
      // MODIFIES: *this
      // EFFECTS: Creates a string with equivalent contents to cstr
      string( const char *cstr )
         {
            if ( cstr )
            {
               size_t length = 0;
               while ( cstr[length] != '\0' )
               {
                  ++length;
               }
               m_size = length;
               m_capacity = m_size + 1;
               m_data = new char[m_capacity];
               for ( size_t i = 0; i < m_size; ++i ) 
               {
                  m_data[i] = cstr[i];
               }
               m_data[m_size] = '\0';
            }
            else
            {
               m_size = 0;
               m_capacity = 1;
               m_data = new char[1];
               *m_data = '\0';
            }
         }


      // string Literal / C string Constructor with length
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Creates a string with length and equivalent contents to cstr
      string( const char *cstr, size_t length )
         {
            if ( cstr )
            {
               m_size = length;
               m_capacity = m_size + 1;
               m_data = new char[ m_capacity ];
               for ( size_t i = 0; i < m_size; ++i ) 
               {
                  m_data[ i ] = cstr[ i ];
               }
               *(m_data + m_size) = '\0';
            }
            else
            {
               m_size = 0;
               m_capacity = 1;
               m_data = new char[1];
               *m_data = '\0';
            }
         }
      
      // string constructor with length
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Creates a string with length
      string( size_t length )
         {

            m_size = length;
            m_capacity = m_size + 1;
            m_data = new char[ m_capacity ];
            *(m_data + m_size) = '\0';
         }


      // Copy Constructor
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Copy the contents of other to this
      string(const string &other) 
         {

            m_size = other.m_size;
            m_capacity = other.m_capacity;
            m_data = new char[ other.m_capacity ];
            memcpy(m_data, other.m_data, m_size + 1);
            *(m_data + m_size) = '\0';
         }

      // c_string converter
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns m_data
      const char *c_str() const
         {
            return m_data;
         }

      // Length member function
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns m_length
      size_t length() const
         {
            return m_size;
         }
      
      // Copy Assignment Operator
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Copy the contents of other to this
      string &operator=(const string &other) 
         {
            if ( this != &other ) // not self-assignment
               {
                  delete[] m_data;

                  m_size = other.m_size;
                  m_capacity = other.m_capacity;
                  m_data = new char[m_size + 1];
                  for ( int i = 0; i < m_size; i ++ )
                     {
                        m_data[i] = other.m_data[i];
                     }
                  *(m_data + m_size) = '\0';
               }
            return *this;
         }


      // Destructor
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: release allocated memory
      ~string() 
         {
            delete[] m_data;
         }

      // Size
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns the number of characters in the string
      size_t size( ) const
         {
            return m_size;
         }

      // Capacity
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns the string's capacity
      size_t capacity( ) const
         {
            return m_capacity;
         }

      // empty
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns whether string is empty
      bool empty( ) const
         {
            return m_size == 0 ? true : false;
         }

      // C string Conversion
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns a pointer to a null terminated C string of *this
      const char *cstr( ) const
         {
            return m_data;
         }

      // Iterator Begin
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns a random access iterator to the start of the string
      const char *begin( ) const
         {
            return m_data;
         }

      // Iterator End
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns a random access iterator to the end of the string
      const char *end( ) const
         {
            return m_data + m_size;
         }

      // Element Access
      // REQUIRES: 0 <= i < size()
      // MODIFIES: Allows modification of the i'th element
      // EFFECTS: Returns the i'th character of the string
      char &operator [ ]( size_t i )
         {
            // assert ( i < m_size );
            return m_data[i];
         }

      // string Append
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Appends the contents of other to *this, resizing any
      //      memory at most once
      void operator+=( const string &other )
         {
            if ( m_size + other.m_size >= m_capacity )
            {
               m_capacity = m_size + other.m_size + 1;
               char* new_data = new char[m_capacity];
               memcpy(new_data, m_data, m_size);
               delete[] m_data;
               m_data = new_data;
            }
            memcpy(m_data + m_size, other.m_data, other.m_size + 1);
            m_size += other.m_size;
         }
      
      string operator+( const string &other )
         {
            string s(*this);
            s += other;
            return s;
         }
      
      string operator+( const char *other )
         {
            string s(*this);
            s += string(other);
            return s;
         }

      // Push Back
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Appends c to the string
      void push_back( char c )
         {
            if ( m_size == m_capacity - 1 )
            {
               m_capacity *= 2;
               char* new_data = new char[m_capacity];
               memcpy(new_data, m_data, m_size);
               delete[] m_data;
               m_data = new_data;
            }
            *(m_data + m_size++) = c;
            *(m_data + m_size) = '\0';
         }

      // Pop Back
      // REQUIRES: string is not empty
      // MODIFIES: *this
      // EFFECTS: Removes the last charater of the string
      void popBack( )
         {
            if ( m_size > 0 )
            {
               *(m_data + --m_size) = '\0';
            }
         }

      // Pop Back
      // REQUIRES: string is not empty
      // MODIFIES: *this
      // EFFECTS: Removes the last n charaters of the string
      void popBack( size_t n )
         {
            while ( m_size > 0 && n > 0)
            {
               *(m_data + --m_size) = '\0';
               --n;
            }
         }

      // Equality Operator
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns whether all the contents of *this
      //    and other are equal
      bool operator==( const string &other ) const
         {
            if ( other.m_size != m_size )
            {
               return false;
            }
            for ( size_t i = 0; i < m_size; ++i )
            {
               if ( m_data[i] != other.m_data[i] )
               {
                  return false;
               }
            }
            return true; 
         }

      // Not-Equality Operator
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns whether at least one character differs between
      //    *this and other
      bool operator!=( const string &other ) const
         {
            return !( *this == other );
         }

      // Less Than Operator
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns whether *this is lexigraphically less than other
      bool operator<( const string &other ) const
         {
            size_t i1 = 0;
            size_t i2 = 0;

            while ( i1 < m_size and i2 < other.m_size )
            {
               if ( m_data[i1] < other.m_data[i2] )
               {
                  return true;
               }
               else if ( m_data[i1] > other.m_data[i2] )
               {
                  return false;
               }
               ++ i1;
               ++ i2;
            }
            return ( i1 == m_size ) && ( i2 < other.m_size );
         }

      // Greater Than Operator
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns whether *this is lexigraphically greater than other
      bool operator>( const string &other ) const
         {
            return !( *this <= other );
         }

      // Less Than Or Equal Operator
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns whether *this is lexigraphically less or equal to other
      bool operator<=( const string &other ) const
         {
            return ( *this < other ) || ( *this == other );
         }

      // Greater Than Or Equal Operator
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns whether *this is lexigraphically less or equal to other
      bool operator>=( const string &other ) const
         {
            return ( *this == other ) || ( *this > other );
         }

      // Assign
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: assign the content of s to this string
      void assign( const char *s, size_t count )
         {
            delete[] m_data;

            m_size = count;
            m_capacity = m_size + 1;
            m_data = new char[ m_capacity ];
            memcpy(m_data, s, m_size);
            *(m_data + m_size) = '\0';

         }
      
      // charcount
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns the number of occurances of char c in the string.
      int charcount( const char c ) const {
         int count = 0;
         for (int i = 0; i < m_size; i++)
            if (m_data[i] == c)
               count++;
         return count;
      }

      // find
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns a the position of *s in the string. If *s is not in the string, returns -1.
      int find( const char *s ) const
         {
            if (!s || !*s) return -1;

            size_t s_length = 0;
            while (s[s_length] != '\0') 
            {
               ++s_length;
            }

            if (s_length > m_size) return -1;

            for (size_t i = 0; i <= m_size - s_length; ++i) {
               size_t j = 0;
               while (j < s_length && m_data[i + j] == s[j]) {
                  ++j;
               }
               if (j == s_length) {
                  return i; 
               }
            }
            return -1; 
         }

      int find( const char *s, size_t pos ) const
         {
            if (!s || !*s) return -1;

            size_t s_length = 0;
            while (s[s_length] != '\0') 
            {
               ++s_length;
            }

            if (s_length > m_size || pos > m_size) return -1;

            for (size_t i = pos; i <= m_size - s_length; ++i) {
               size_t j = 0;
               while (j < s_length && m_data[i + j] == s[j]) {
                  ++j;
               }
               if (j == s_length) {
                  return i; 
               }
            }
            return -1; 
         }

      char *at( size_t pos ) const 
         {
            return m_data + pos;
         }

      char &operator[ ]( size_t i ) const 
         {
            return *(m_data + i);
         }

      // Substring
      // REQUIRES: pos <= size() and count is a valid size
      // MODIFIES: Nothing
      // EFFECTS: Returns a substring starting at pos with length count
      string substr(size_t pos, size_t count) const {
         if (pos > m_size) {
            return string();
         }
         if (pos + count > m_size) {
            count = m_size - pos;
         }
         return string(m_data + pos, count);
      }


      // Returns all data after pos
      string substr(size_t pos) const {
         if (pos > m_size) {
            return string();
         }
         return string(m_data + pos, m_size-pos);
      }
      // Overload the + operator
      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns a new string that is the concatenation of *this and other
      string operator+(const string &other) const {
         string result;
         result.m_size = m_size + other.m_size;
         result.m_capacity = result.m_size + 1;
         result.m_data = new char[result.m_capacity];

         memcpy(result.m_data, m_data, m_size);

         memcpy(result.m_data + m_size, other.m_data, other.m_size);
         
         *(result.m_data + result.m_size) = '\0';
         return result;
      }

      char * data() const{
         return m_data;
      }

      friend std::istream &operator>>( std::istream &is, string &s );

   private:
      size_t m_size;
      size_t m_capacity;
      char *m_data = nullptr;
   };

std::ostream &operator<<( std::ostream &os, const string &s );