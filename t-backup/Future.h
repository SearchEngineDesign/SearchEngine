#pragma once

template <typename T>
class Future {
private:
    struct SharedState {
        T result;
        bool ready = false;
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    };

    SharedState* state;

    
    static void execute(void* arg) {
        auto* stateFuncPair = static_cast<std::pair<SharedState*, T(*)()>*>(arg);
        SharedState* state = stateFuncPair->first;
        T (*func)() = stateFuncPair->second;
        delete stateFuncPair;  // delete because it was allocated in the constructor

        T result = func();

        pthread_mutex_lock(&state->mutex);
        state->result = result;
        state->ready = true;
        pthread_cond_signal(&state->cond);
        pthread_mutex_unlock(&state->mutex);
    }

public:
    Future(ThreadPool& pool, T (*func)()) {
        state = new SharedState;
        auto* arg = new std::pair<SharedState*, T(*)()>(state, func);

        // put the function in the thread pool
        pool.submit(execute, arg);
    }

    // Future(T (*func)()) {
    //     state = new SharedState;
    //     auto* arg = new std::pair<SharedState*, T(*)()>(state, func);

    //     // put the function in a thread
    //     pthread_t thread;
    //     pthread_create(&thread, nullptr, execute, arg);
    //     pthread_detach(thread);  // detach the thread to avoid memory leaks
    // }



    bool ready() {
        pthread_mutex_lock(&state->mutex);
        bool isReady = state->ready;
        pthread_mutex_unlock(&state->mutex);
        return isReady;
    }

    // block until the result is ready 
    T get() {
        pthread_mutex_lock(&state->mutex);
        while (!state->ready) {
            pthread_cond_wait(&state->cond, &state->mutex);
        }
        T result = state->result;
        pthread_mutex_unlock(&state->mutex);
        return result;
    }

    ~Future() {
        pthread_mutex_destroy(&state->mutex);
        pthread_cond_destroy(&state->cond);
        delete state;
    }
};

