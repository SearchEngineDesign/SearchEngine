// vector.h
// 
// Starter file for a vector template
#pragma once
#include <utility>
#include <initializer_list>
#include <iostream>

template<typename T>
   class vector
   {
   public:

      // Default Constructor
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Constructs an empty vector with capacity 0
      vector( )
         {
		      reset();
         }

      // Destructor
      // REQUIRES: Nothing
      // MODIFIES: Destroys *this
      // EFFECTS: Performs any neccessary clean up operations
      ~vector( )
         {
            delete[] arr;              //deallocate arr[]
         }

      // Resize Constructor
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Constructs a vector with size num_elements,
      //    all default constructed
      vector( size_t num_elements )
         {
            if (num_elements == 0)
            {
               reset();
            }
            else
            {
               vecSize = num_elements;
               vecCapacity = num_elements;  
               arr = new T[vecCapacity];   
               for ( size_t i = 0; i < num_elements; i++ ) 
                  arr[ i ] = T( );
            }
         }

      // Fill Constructor
      // REQUIRES: Capacity > 0
      // MODIFIES: *this
      // EFFECTS: Creates a vector with size num_elements, all assigned to val
      vector( size_t num_elements, const T &val )
         {
            if (num_elements == 0)
            reset();
            else
            {
               vecSize = num_elements;
               vecCapacity = num_elements;
               arr = nullptr;
               if ( num_elements > 0 )
                  {
                     arr = new T[ num_elements ];
                     for ( size_t i = 0; i < num_elements; i++ )
                        arr[ i ] = val;
                  }
            }
         }

      // Copy Constructor
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Creates a clone of the vector other
      vector( const vector<T> &other )
         {
            vecSize = other.vecSize;
            vecCapacity = other.vecCapacity;
            arr = nullptr;
            if ( vecSize > 0 )
               {
                  arr = new T[ vecCapacity ];
                  for ( size_t i = 0; i < vecSize; i++ )
                     arr[ i ] = other.arr[ i ];
               }
         }

      // Assignment operator
      // REQUIRES: Nothing
      // MODIFIES: *this
      // EFFECTS: Duplicates the state of other to *this
      vector operator=( const vector<T> &other )
         {
            if ( this != &other )
               {
                  delete[] arr;
                  vecSize = other.vecSize;
                  vecCapacity = other.vecCapacity;
                  arr = nullptr;
                  if ( vecSize > 0 )
                     {
                        arr = new T[ vecCapacity ];
                        for ( size_t i = 0; i < vecSize; i++ )
                           arr[ i ] = other.arr[ i ];
                     }
               }
            return *this;
         }

      // Move Constructor
      // REQUIRES: Nothing
      // MODIFIES: *this, leaves other in a default constructed state
      // EFFECTS: Takes the data from other into a newly constructed vector
      vector( vector<T> &&other )
         {
            vecSize = other.vecSize;
            vecCapacity = other.vecCapacity;
            arr = other.arr;
            other.vecSize = 0;
            other.vecCapacity = 0;
            other.arr = nullptr;
         }

      // Move Assignment Operator
      // REQUIRES: Nothing
      // MODIFIES: *this, leaves otherin a default constructed state
      // EFFECTS: Takes the data from other in constant time
      vector operator=( vector<T> &&other )
         {
            if ( this != &other )
               {
                  delete[] arr;
                  vecSize = other.vecSize;
                  vecCapacity = other.vecCapacity;
                  arr = other.arr;
                  other.vecSize = 0;
                  other.vecCapacity = 0;
                  other.arr = nullptr;
               }
            return *this;
         }

      // REQUIRES: new_capacity > capacity( )
      // MODIFIES: capacity( )
      // EFFECTS: Ensures that the vector can contain size( ) = new_capacity
      //    elements before having to reallocate
      void reserve( size_t newCapacity )
         {  
            if ( newCapacity > vecCapacity )
            {
               vecCapacity = newCapacity;
               T* newArr = new T[ newCapacity ];
               for ( size_t i=0; i < vecSize; i++ )
                  {
                     newArr[ i ] = arr[ i ];
                  }
               delete[] arr;
               arr = newArr;
            }
         }

      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns the number of elements in the vector
      size_t size( ) const
         {
            return this->vecSize;
         }

      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns the maximum size the vector can attain before resizing
      size_t capacity( ) const
         {
            return this->vecCapacity;
         }

      // REQUIRES: 0 <= i < size( )
      // MODIFIES: Allows modification of data[i]
      // EFFECTS: Returns a mutable reference to the i'th element
      T &operator[ ]( size_t i )
         {
            if (i >= this->vecSize)
               throw std::runtime_error("index out of range");
            return this->arr[i];
         }

      // REQUIRES: 0 <= i < size( )
      // MODIFIES: Nothing
      // EFFECTS: Get a const reference to the ith element
      const T &operator[ ]( size_t i ) const
         {
            if (i >= this->vecSize)
               throw std::runtime_error("index out of range");
            return this->arr[i];
         }

      // REQUIRES: Nothing
      // MODIFIES: this, size( ), capacity( )
      // EFFECTS: Appends the element x to the vector, allocating
      //    additional space if neccesary
      void push_back( const T &x )
         {
            if (this->vecSize == this->vecCapacity) {
               if ( this->vecCapacity != 0 )
               {
                  reserve(this->vecCapacity * 2);
               }
               else
               {
                  reserve( this->vecCapacity + 1 );
               }
            }
            arr[this->vecSize] = x;
            ++this->vecSize;

         }

      // REQUIRES: Nothing
      // MODIFIES: this, size( )
      // EFFECTS: Removes the last element of the vector,
      //    leaving capacity unchanged
      void popBack( )
         {
            if (this->vecSize == 0)
               throw std::runtime_error("popBack on empty vector");
            // arr[this->vecSize - 1] = T();
            --this->vecSize;
         }

      void resize( size_t new_count, const T& new_value ) 
         {
            if (new_count < vecSize)
               {
                  while ( vecSize != new_count )
                     popBack();
               } else if ( new_count > vecSize ) {
                  while ( vecSize != new_count )
                     push_back(new_value);
               }
         }

      // REQUIRES: Nothing
      // MODIFIES: Allows mutable access to the vector's contents
      // EFFECTS: Returns a mutable random access iterator to the 
      //    first element of the vector
      T* begin( )
         {
            return arr;
         }

      // REQUIRES: Nothing
      // MODIFIES: Allows mutable access to the vector's contents
      // EFFECTS: Returns a mutable random access iterator to 
      //    one past the last valid element of the vector
      T* end( )
         {
            return arr + vecSize;
         }

      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Rxeturns a random access iterator to the first element of the vector
      const T* begin( ) const
         {
            return arr;
         }

      // REQUIRES: Nothing
      // MODIFIES: Nothing
      // EFFECTS: Returns a random access iterator to 
      //    one past the last valid element of the vector
      const T* end( ) const
         {
            return arr + vecSize;
         }
      
      T& back() const
         {
            if (vecSize == 0)
               throw std::runtime_error("back on empty vector");
            return arr[vecSize - 1];
         }
      
      T& front() const
         {
            if (vecSize == 0)
               throw std::runtime_error("front on empty vector");
            return arr[0];
         }

      T* data() 
         {
            return arr;
         }
      
      bool empty() const
         {
            return vecSize == 0;
         }
      
      void clear()
         {
            delete[] arr;  
            vecSize = 0;
            vecCapacity = 0;
            arr = nullptr;
         }

      template<typename... Args>
      void emplace_back(Args&&... args) 
         {
         if ( this->vecSize >= this->vecCapacity ) 
            {
            if ( this->vecCapacity == 0 ) 
               reserve( this->vecCapacity + 1 );
            else 
               reserve( this->vecCapacity * 2 );
            }
         new(this->arr + this->vecSize) T( std::forward<Args>(args)... );
         this->vecSize++;
         }

   private:
      void reset() {
         delete[] arr;
         this->vecSize = 0;         //size = 0, there is nothing in the vector
         this->vecCapacity = 0;
         this->arr = nullptr;   
      }

      size_t vecSize;
      size_t vecCapacity;
      T* arr = nullptr;

   };