#pragma once


#include <cf/threading/cfmutex.h>

namespace cf {
    class Guard {
        private:
            Mutex& mutex;
        public:
            Guard(Mutex& m) : mutex(m) {
                mutex.lock();
            }
            ~Guard() {
                mutex.unlock();
            }
    };
}