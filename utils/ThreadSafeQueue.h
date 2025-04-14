#pragma once


#include <queue>

#include <pthread.h>


template<typename T> 
class ThreadSafeQueue {
    
    private:
        std::queue<T> queue; 
        pthread_mutex_t mutex; 
        pthread_cond_t cond;

    public:
        ThreadSafeQueue() {
            pthread_mutex_init(&mutex, nullptr);
            pthread_cond_init(&cond, nullptr);
        }

        void put(const T& item) {
            pthread_mutex_lock(&mutex);
            queue.push(item);
            pthread_cond_broadcast(&cond); // Notify ALL(?) waiting thread
            pthread_mutex_unlock(&mutex);
        }

        T get() {
            pthread_mutex_lock(&mutex);
            while (queue.empty()) {
                pthread_cond_wait(&cond, &mutex); // Wait for an item to be available
            }
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

        ~ThreadSafeQueue() {
            pthread_mutex_destroy(&mutex);
            pthread_cond_destroy(&cond);
        }
};



