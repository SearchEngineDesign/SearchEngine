#pragma once

#include <pthread.h>

namespace cf {
    class Mutex {
        private: 
            pthread_mutex_t mutex;
        public:
            Mutex() {
                pthread_mutex_init(&mutex, nullptr);
            }
            ~Mutex() {
                pthread_mutex_destroy(&mutex);
            }
            void lock() {
                pthread_mutex_lock(&mutex);
            }
            void unlock() {
                pthread_mutex_unlock(&mutex);
            }
            bool tryLock() {
                return pthread_mutex_trylock(&mutex) == 0;
            }
    };
}
