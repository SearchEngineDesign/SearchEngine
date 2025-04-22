#pragma once


#include <queue>

#include <pthread.h>


template<typename T> 
class ThreadSafeQueue {
    
    private:
        std::queue<T> queue; 
        pthread_mutex_t mutex; 
        pthread_cond_t cond;

        bool kill = false;

        static const int MAX_SIZE = 40000;

    public:
        ThreadSafeQueue() {
            pthread_mutex_init(&mutex, nullptr);
            pthread_cond_init(&cond, nullptr);
        }

        void put(const T& item, bool blocking) {
            pthread_mutex_lock(&mutex);
            queue.push(item);
            pthread_cond_broadcast(&cond); // Notify ALL(?) waiting thread
            if (blocking)
                while (queue.size() > MAX_SIZE)
                    pthread_cond_wait(&cond, &mutex);
            pthread_mutex_unlock(&mutex);
        }

        T get() {
            pthread_mutex_lock(&mutex);
            while (queue.empty() && !kill)
                pthread_cond_wait(&cond, &mutex); // Wait for an item to be available
            T item = queue.front();
            queue.pop();
            pthread_mutex_unlock(&mutex);
            return item;
        }

        bool empty() {
            pthread_mutex_lock(&mutex);
            bool isEmpty = queue.empty();
            pthread_mutex_unlock(&mutex);
            return isEmpty;
        }

        void emptyQueue() {
            pthread_mutex_lock(&mutex);
            std::queue<T>().swap(queue); // Clear the queue
            pthread_mutex_unlock(&mutex);
        }

        void stop() {
            kill = true;
            std::cout << "Freeing queue..." << std::endl;
            pthread_cond_broadcast(&cond);
        }

        void wait() {
            pthread_mutex_lock(&mutex);
            while (queue.size() > MAX_SIZE)
                pthread_cond_wait(&cond, &mutex); 
            pthread_mutex_unlock(&mutex);
        }

        ~ThreadSafeQueue() {
            pthread_mutex_destroy(&mutex);
            pthread_cond_destroy(&cond);
        }

        uint32_t size() {
            return queue.size();
        }
};



