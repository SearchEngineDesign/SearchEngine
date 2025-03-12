#pragma once

// HashBlob, a serialization of a HashTable into one contiguous
// block of memory, possibly memory-mapped to a HashFile.

// Nicole Hamilton  nham@umich.edu

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <cstdint>
#include <stdlib.h> //instead of #include <malloc.h> DZ
#include <unistd.h>
#include <sys/mman.h>

#include "HashTable.h"


using Hash = HashTable< const char *, size_t >;
using Pair = Tuple< const char *, size_t >;
using HashBucket = Bucket< const char *, size_t >;


static const size_t Unknown = 0;


size_t RoundUp( size_t length, size_t boundary )
   {
   // Round up to the next multiple of the boundary, which
   // must be a power of 2.

   static const size_t oneless = boundary - 1,
      mask = ~( oneless );
   return ( length + oneless ) & mask;
   }


struct SerialTuple
   {
   // This is a serialization of a HashTable< char *, size_t >::Bucket.
   // One is packed up against the next in a HashBlob.

   // Since this struct includes size_t and uint32_t members, we'll
   // require that it be sizeof( size_t ) aligned to avoid unaligned
   // accesses.

   public:

      // SerialTupleLength = 0 is a sentinel indicating
      // this is the last SerialTuple chained in this list.
      // (Actual length is not given but not needed.)

      size_t Length, Value;

      // The Key will be a C-string of whatever length.
      char Key[ Unknown ];

      // Calculate the bytes required to encode a HashBucket as a
      // SerialTuple.

      static size_t BytesRequired( const HashBucket *b )
         {
         //Value, Length
         size_t size = sizeof(size_t) * 2;
         //string size. 
         size += strlen(b->tuple.key) + sizeof(char); //why sizeof char?
         return RoundUp(size, sizeof(size_t)); //not sure if this is accurate
         }

      // Write the HashBucket out as a SerialTuple in the buffer,
      // returning a pointer to one past the last character written.

      static char *Write( char *buffer, size_t len,
            const HashBucket *b )
         {
         SerialTuple* t = reinterpret_cast<SerialTuple*>(buffer);
         t->Length = len;
         t->Value = b->tuple.value;
         strcpy(t->Key, b->tuple.key);
         return (buffer + len + 1);
         }
  };


class HashBlob
   {
   // This will be a hash specifically designed to hold an
   // entire hash table as a single contiguous blob of bytes.
   // Pointers are disallowed so that the blob can be
   // relocated to anywhere in memory

   // The basic structure should consist of some header
   // information including the number of buckets and other
   // details followed by a concatenated list of all the
   // individual lists of tuples within each bucket.

   public:

      // Define a MagicNumber and Version so you can validate
      // a HashBlob really is one of your HashBlobs.

      size_t MagicNumber,
         Version,
         BlobSize,
         NumberOfBuckets,
         Buckets[ Unknown ];

      // The SerialTuples will follow immediately after.


      const SerialTuple *Find( const char *key ) const
         {
         // Search for the key k and return a pointer to the
         // ( key, value ) entry.  If the key is not found,
         // return nullptr.

         // Your code here.
         size_t index = Hash::hashbasic(key, NumberOfBuckets);
         size_t bucketStart = Buckets[index];
         SerialTuple *curr = reinterpret_cast<SerialTuple*>((char *)this + bucketStart);

         size_t bucketEnd;
         (index == NumberOfBuckets - 1) ? bucketEnd = BlobSize : bucketEnd = Buckets[index+1];

         while (bucketStart < bucketEnd)
         {
            if (!strcmp(curr->Key, key))
            {
               return curr;
            } else {
               bucketStart += curr->Length;
               curr = reinterpret_cast<SerialTuple*>((char *)this + bucketStart);
            }
            
         }

         return nullptr;
         }


      static size_t BytesRequired( const Hash *hashTable, size_t &offset )
         {
         // Calculate how much space it will take to
         // represent a HashTable as a HashBlob.

         // Need space for the header + buckets +
         // all the serialized tuples.

         //space for 2048 offsets of size size_t + the 4 hashblob members
         offset = (hashTable->size()+4) * sizeof(size_t);
         size_t bucketSpace = 0;
         for (int i = 0; i < hashTable->size(); i++)
            {
               HashBucket *curr = hashTable->at(i);
               while (curr != nullptr)
               {
                  bucketSpace += SerialTuple::BytesRequired(curr);
                  curr = curr->next;
               }
            }

         return offset + bucketSpace;
         }

      // Write a HashBlob into a buffer, returning a
      // pointer to the blob.

      static HashBlob *Write( HashBlob *hb, size_t bytes,
            const Hash *hashTable )
         {
         // Your code here.
         // What does this do ???

         return nullptr;
         }

      // Create allocates memory for a HashBlob of required size
      // and then converts the HashTable into a HashBlob.
      // Caller is responsible for discarding when done.

      // (No easy way to override the new operator to create a
      // variable sized object.)

      static HashBlob *Create( const Hash *hashTable )
         {
         //allocating the whole blob
         size_t offset;
         size_t size = HashBlob::BytesRequired( hashTable, offset );
         char *mem = new char[size];
         HashBlob *blob = reinterpret_cast<HashBlob*>(mem);

         blob->MagicNumber = 15276156;
         blob->Version = 1;
         blob->BlobSize = size;
         blob->NumberOfBuckets = hashTable->size();

         size_t *q;
         for (int i = 0; i < hashTable->size(); i++)
         {
            blob->Buckets[i] = offset;
            HashBucket *curr = hashTable->at(i);
            
            while (curr != nullptr) {
               size_t tSize = SerialTuple::BytesRequired(curr);
               SerialTuple::Write(mem + offset, tSize, curr);
               offset += tSize;
               if (!strcmp(curr->tuple.key, "<title>Introduction</title>"))
                  q = blob->Buckets + i;
               curr = curr->next;
            }
            
         }
         
         return blob;
         }

      // Discard

      static void Discard( HashBlob *blob )
         {
         // Your code here.
         }
   };

class HashFile
   {
   private:

      int fd;
      HashBlob *blob;
      struct stat fileInfo;

      size_t FileSize( int f )
         {
         fstat( f, &fileInfo );
         return fileInfo.st_size;
         }

   public:

      const HashBlob *Blob( )
         {
         return blob;
         }

      HashFile( const char *filename )
         {
            // Open the file for reading, map it, check the header,
            // and note the blob address.

            // Your code here.
            fd = open(filename, O_RDONLY); //open file
            if (fd == -1) 
               perror("open");

            if (FileSize(fd) == -1) //get file size
               perror("fstat");

            blob = reinterpret_cast<HashBlob*>(mmap(nullptr, fileInfo.st_size, 
                                       PROT_READ, MAP_PRIVATE, fd, 0)); //map bytes to 'blob'
            if (blob == MAP_FAILED)
               perror("mmap");

         }

      HashFile( const char *filename, const Hash *hashtable )
         {
         // Open the file for write, map it, write
         // the hashtable out as a HashBlob, and note
         // the blob address.

         // Your code here.
         }

      ~HashFile( )
         {
            munmap(blob, fileInfo.st_size);  //not sure if this should happen here or earlier
            close(fd);                       //not sure if this should happen here or earlier
         }
   };
