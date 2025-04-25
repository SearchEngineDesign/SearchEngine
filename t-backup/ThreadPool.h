#include <pthread.h>
#include <queue>
#include <cf/vec.h>
#pragma once


#define THREAD_POOL_SIZE 10

struct Task {
    void (*func)(void*);  
    void* arg;            
};

class ThreadPool {
private:
    std::vector<pthread_t> workers;
    std::queue<Task> taskQueue;
    pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t queueCond = PTHREAD_COND_INITIALIZER;
    bool stop = false;

    size_t numThreads;

    // Worker thread function
    static void* workerThread(void* arg) {
        ThreadPool* pool = static_cast<ThreadPool*>(arg);

        while (true) {
            pthread_mutex_lock(&pool->queueMutex);

            // Wait for a task
            while (pool->taskQueue.empty() && !pool->stop) {
                pthread_cond_wait(&pool->queueCond, &pool->queueMutex);
            }

            // Stop signal
            if (pool->stop && pool->taskQueue.empty()) {
                pthread_mutex_unlock(&pool->queueMutex);
                break;
            }

            // Get task from queue
            Task task = pool->taskQueue.front();
            pool->taskQueue.pop();

            pthread_mutex_unlock(&pool->queueMutex);

            // Execute task
            task.func(task.arg);
        }
        return nullptr;
    }

public:
    ThreadPool(size_t numThreads = THREAD_POOL_SIZE): numThreads(numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            pthread_t thread;
            pthread_create(&thread, nullptr, workerThread, this);
            workers.push_back(thread);
        }
    }

    void submit(void (*func)(void*), void* arg) {
        pthread_mutex_lock(&queueMutex);
        taskQueue.push(Task{func, arg});
        pthread_cond_signal(&queueCond);
        pthread_mutex_unlock(&queueMutex);
    }

    void shutdown() {

        pthread_mutex_lock(&queueMutex);
        stop = true;
        // std::queue<Task>().swap(taskQueue);

        
        pthread_cond_broadcast(&queueCond);
        pthread_mutex_unlock(&queueMutex);     
        
        for (pthread_t& thread : workers) {
            pthread_join(thread, nullptr);
        }

    }

    ~ThreadPool() {

        shutdown();

        pthread_mutex_destroy(&queueMutex);
        pthread_cond_destroy(&queueCond);
        
    }
};

