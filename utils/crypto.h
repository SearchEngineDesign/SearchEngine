#pragma once

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <utility>
#include "../utils/string.h"
#include <cassert>

#include "../frontier/ReaderWriterLock.h"


class Crypto {
    
    private:
      EVP_MD_CTX* ctx = nullptr;
        ReaderWriterLock lock;
    public:

    Crypto() {
        // std::cout << "make crpypto ctx from " << caller << std::endl;
        ctx = EVP_MD_CTX_new();
    }


        std::pair<uint64_t, uint64_t> doubleHash(const string &datum) {
            
            WithWriteLock l(lock);
            

            assert(datum.length() > 0);

            const EVP_MD* md = EVP_sha256();
            if (!md) {
                throw std::runtime_error("EVP_sha256 returned NULL");
            }


            unsigned char hash_digest[EVP_MAX_MD_SIZE];
        
            EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
            EVP_DigestUpdate(ctx, datum.c_str(), datum.length());
            EVP_DigestFinal_ex(ctx, hash_digest, nullptr);
   
   
            // Split the 128-bit hash into two 64-bit parts
            uint64_t hash1 = 0, hash2 = 0;
            std::memcpy(&hash1, hash_digest, sizeof(uint64_t));
            std::memcpy(&hash2, hash_digest + sizeof(uint64_t), sizeof(uint64_t));
        
            return {hash1, hash2};
        
        }




        size_t hashMod(const string& datum, size_t size) {

            WithWriteLock l(lock);

            unsigned char hash_digest[EVP_MAX_MD_SIZE];
            

            EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
            EVP_DigestUpdate(ctx, datum.c_str(), datum.length());
            EVP_DigestFinal_ex(ctx, hash_digest, nullptr);

            __uint128_t hash;
            std::memcpy(&hash, hash_digest, sizeof(__uint128_t));


            return (hash % size);
        }

        ~Crypto() {
            EVP_MD_CTX_free(ctx);
            ctx = nullptr;
        }

};