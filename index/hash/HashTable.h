// Simple hash table template.

// Nicole Hamilton  nham@umich.edu

#pragma once

#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <algorithm>

static const size_t initialSize = 2048;

// You may add additional members or helper functions.


// Compare C-strings, return true if they are the same.

bool CompareEqual( const char *L, const char *R );

template< typename Key, typename Value > class Tuple
   {
   public:
      Key key;
      Value value;

      Tuple( const Key &k, const Value v ) : key( k ), value( v )
         {
         }
   };


template< typename Key, typename Value > class Bucket
   {
   public:
      Bucket *next;
      Tuple< Key, Value > tuple;

      Bucket( const Key k, const Value v ) :
            tuple( k, v ), next( nullptr )
         {
         }
   };

template< typename Key, typename Value > 
class CompareBuckets {
public:
   bool operator()(const Bucket<Key, Value> *a, const Bucket<Key, Value> *b) const {
      if (a == nullptr)
         return false;
      if (b == nullptr)
         return true;
      return a->tuple.value > b->tuple.value; 
   }
};

template< typename Key, typename Value > class HashTable
   {
   private:
   friend class HashBlob;

      // Your code here.

      Bucket< Key, Value > **buckets;
      size_t numberOfBuckets = initialSize;
      const double seed = ((double) rand() / (RAND_MAX));

      friend class Iterator;
      friend class TopN;
      friend class HashBlob;

      char **optBuffer;
      size_t bufSize = 0;
      char **optBufferRef;

   public:
      static size_t hashbasic(const char *c, const size_t &mod) 
         {
            size_t i = 0;
            
            while(*c != '\0')
            {
               i += *c;
               c++;
            }
            return i % mod;
         }
      size_t size() const
         {
            return numberOfBuckets;
         }
      Bucket <Key, Value> *at(size_t index) const
         {
            return buckets[index];
         }

      Tuple< Key, Value > *Find( const Key k, const Value initialValue )
         {

            size_t index = hashbasic(k.at(0), numberOfBuckets);
            Bucket< Key, Value > *curr = buckets[index];
            Bucket< Key, Value > *prev = curr;
            
            while (curr != nullptr)
            {
               if (k == curr->tuple.key) 
                  return &(curr->tuple);
               prev = curr;
               curr = curr->next;
            }
            curr = new Bucket< Key, Value >(k, initialValue);
            if (prev) 
               prev->next = curr;
            else
               buckets[index] = curr;
            return &(curr->tuple);
         }

      Tuple< Key, Value > *Find( const Key k ) const
         {
            size_t index = hashbasic(k.at(0), numberOfBuckets);
            Bucket< Key, Value > *curr = buckets[index];
            
            while (curr != nullptr)
            {
               if (k == curr->tuple.key) //maybe compare hash values instead
                  return &(curr->tuple);
               curr = curr->next;
            }
            return nullptr;
         }

      //optimize: sorts each bucket by descending order.
      void Optimize( double loading = 1.5 )
         {
            //for ( int i = 0; i < numberOfBuckets; i++)
            //   buckets[i] = MergeSort(buckets[i]); 
         }

      void topbuckets( double loading = 1.5 )
         {
            for ( int i = 0; i < numberOfBuckets; i++)
               buckets[i] = MergeSort(buckets[i]); 
         }

      Bucket< Key, Value> *split(Bucket< Key, Value> *head)
         {
            Bucket< Key, Value> *fast = head;
            Bucket< Key, Value> *slow = head;

            // Move fast pointer two steps and slow pointer
            // one step until fast reaches the end
            while (fast != nullptr && fast->next != nullptr) {
               fast = fast->next->next;
               if (fast != nullptr) {
                     slow = slow->next;
               }
            }

            Bucket< Key, Value> *temp = slow->next;
            slow->next = nullptr;
            return temp;
         }

      // Function to merge two sorted singly linked lists
      Bucket< Key, Value> *merge(Bucket< Key, Value> *first, Bucket< Key, Value> *second) 
         {
            if (first == nullptr) return second;
            if (second == nullptr) return first;

            if (first->tuple.value > second->tuple.value) 
            {
               first->next = merge(first->next, second);
               return first;
            }
            else 
            {
               second->next = merge(first, second->next);
               return second;
            }
         }

      // Function to perform merge sort on a singly linked list
      Bucket< Key, Value> *MergeSort(Bucket< Key, Value> *head) 
         {
            if (head == nullptr || head->next == nullptr)
               return head;

            // Split the list into two halves
            Bucket< Key, Value> *second = split(head);

            // Recursively sort each half
            head = MergeSort(head);
            second = MergeSort(second);

            // Merge the two sorted halves
            return merge(head, second);
         }

      HashTable( )
         {
            numberOfBuckets = initialSize;
            buckets = new Bucket<Key, Value>*[numberOfBuckets];
            for (int i = 0; i < numberOfBuckets; i++)
               {
               buckets[i] = nullptr;
               }
         }

      HashTable( size_t init )
         {
            numberOfBuckets = init * 2;
            buckets = new Bucket<Key, Value>*[numberOfBuckets];
            for (int i = 0; i < numberOfBuckets; i++)
               {
               buckets[i] = nullptr;
               }
         }


      ~HashTable( )
         {
         if( buckets == nullptr )
            return;
         for( size_t i = 0; i < numberOfBuckets; i++ )
            {
            Bucket<Key, Value> *curr = buckets[i];
            Bucket<Key, Value> *next = nullptr;
            while ( curr != nullptr )
               {
               next = curr->next;
               delete curr;
               curr = next;
               }
            }
            delete[] buckets;
            buckets = nullptr;
         }


         class Iterator
         {
         private:

            friend class HashTable;

            HashTable *currTable;
            size_t index;
            Bucket<Key, Value> *bucket;

            Iterator( HashTable *table, size_t bucket, Bucket<Key, Value> *b ) : //what to do with bucket sizet?
               index(bucket), currTable(table), bucket(b) {}

         public:

            Iterator( ) : Iterator( nullptr, 0, nullptr )
               {
               }

            ~Iterator( )
               {
               }

            Tuple< Key, Value > &operator*( )
               {
                  return bucket->tuple;
               }

            Tuple< Key, Value > *operator->( ) const
               {
                  return &(bucket->tuple);
               }

            // Prefix ++
            Iterator &operator++( )
               {
                  if (bucket->next == nullptr) {
                     index++;
                     for (; index < currTable->numberOfBuckets; index++) 
                     {
                        if (currTable->buckets[index] != nullptr)
                        {
                           bucket = currTable->buckets[index];
                           break;
                        }
                     }
                  } else {
                     bucket = bucket->next;
                  }
               }

            // Postfix ++
            Iterator operator++( int )
               {
                  if (bucket->next == nullptr) {
                     index++;
                     for (; index < currTable->numberOfBuckets; index++) 
                     {
                        if (currTable->buckets[index] != nullptr)
                        {
                           bucket = currTable->buckets[index];
                           break;
                        }
                     }
                  } else {
                     bucket = bucket->next;
                  }
               }

            bool operator==( const Iterator &rhs ) const
               {
                  return (rhs.bucket == bucket 
                           && rhs.index == index 
                           && rhs.currTable == currTable);
               }

            bool operator!=( const Iterator &rhs ) const
               {
                  return !(rhs.bucket == bucket 
                           && rhs.index == index 
                           && rhs.currTable == currTable);
               }
         };

      Iterator begin( )
         {
            if (numberOfBuckets != 0)
               return Iterator(this, 0, buckets[0]);
            else
               return end();
         }

      Iterator end( )
         {
            return nullptr;
         }
   };
