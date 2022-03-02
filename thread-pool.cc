/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include <iostream>
#include <thread>
#include <vector>
#include "thread-pool.h"

using namespace std;

using develop::ThreadPool;

/**
 * @brief Construct a new Thread Pool:: Thread Pool object
 */
ThreadPool::ThreadPool(size_t numThreads): wts(numThreads),  all_workers_sema(numThreads), isProgramComplete(false), remaining_thunks(0) {
    dt = thread(&ThreadPool::dispatcher, this);
}

/**
 * @brief spawns new threads as needed to assign a worker a function provided by the 
 * client by calling the schedule method
 */
void ThreadPool::dispatcher() {
    while (true){
        thunk_sema.wait();
        all_workers_sema.wait();
        if (isProgramComplete) break;
        for (size_t workerID = 0; workerID < wts.size(); workerID++) { // guaranteed there is at least one available worker and one thunk in the queue
            worker_info& worker = wts[workerID];
            lock_guard<mutex> lock(worker_field_lock); // prevents the situation where two threads checks the condition at the same time
            if (worker.is_available) {
                worker.is_available = false;
                if (!worker.is_in_use) {
                    worker.is_in_use = true;
                    worker.worker_thread = thread(&ThreadPool::worker, this, workerID);
                }
                lock_guard<mutex> lock_two(queue_locker);
                worker.thunk = thunks_queue.front();
                thunks_queue.pop();
                worker.worker_sema.signal();
                break;
            }
        }
    }
}

/**
 * @brief executes the function assigned by the dispatcher method. Notifies wait if the function
 * was the last one and marks itself as available after finishing the task
 */
void ThreadPool::worker(size_t workerID) {
    while (true){
        wts[workerID].worker_sema.wait();
        if (isProgramComplete) break;
        wts[workerID].thunk();
        worker_field_lock.lock();
        wts[workerID].is_available = true;
        worker_field_lock.unlock();
        all_workers_sema.signal();
        lock_guard<mutex> lock(remaining_thunks_lock);
        remaining_thunks--;
        if (!remaining_thunks) all_thunks_completed.notify_all(); 
    } 
}

/**
 * @brief public method. notifies the dispatcher that there is a new function
 * to execute
 */
void ThreadPool::schedule(const std::function<void(void)>& thunk) {
    queue_locker.lock();
    thunks_queue.push(thunk); 
    queue_locker.unlock();
    lock_guard<mutex> lock(remaining_thunks_lock);
    remaining_thunks++;
    thunk_sema.signal();
}


/**
 * @brief public method that wait all the running thread to finish
 */
void ThreadPool::wait() {
    lock_guard<mutex> lock(remaining_thunks_lock);
    all_thunks_completed.wait(remaining_thunks_lock, [this] {return remaining_thunks == 0; });
}

/**
 * @brief Destroy the Thread Pool:: Thread Pool object
 * 
 */
ThreadPool::~ThreadPool() {
    wait();
    isProgramComplete = true;
    thunk_sema.signal();
    for (auto &worker: wts) {
        if (worker.is_in_use) worker.worker_sema.signal(); // only signal the workers in use
    }
    dt.join();
    for (auto &worker: wts) {
        if (worker.is_in_use) worker.worker_thread.join(); // only join the workers in use
    }  
}

