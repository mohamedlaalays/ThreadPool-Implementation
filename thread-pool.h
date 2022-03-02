/**
 * File: thread-pool.h
 * -------------------
 * Exports a ThreadPool abstraction, which manages a finite pool
 * of worker threads that collaboratively work through a sequence of tasks.
 * As each task is scheduled, the ThreadPool waits for at least
 * one worker thread to be free and then assigns that task to that worker.  
 * Threads are scheduled and served in a FIFO manner, and tasks need to
 * take the form of thunks, which are zero-argument thread routines.
 */

#ifndef _thread_pool_
#define _thread_pool_

#include <cstdlib>
#include <functional>
#include <thread>
#include <semaphore.h>
#include <stdbool.h>
#include <queue>
using namespace std;
// place additional #include statements here

namespace develop {

class ThreadPool {
 public:

/**
 * Constructs a ThreadPool configured to spawn up to the specified
 * number of threads.
 */
  ThreadPool(size_t numThreads);

/**
 * Destroys the ThreadPool class
 */
  ~ThreadPool();

/**
 * Schedules the provided thunk (which is something that can
 * be invoked as a zero-argument function without a return value)
 * to be executed by one of the ThreadPool's threads as soon as
 * all previously scheduled thunks have been handled.
 */
  void schedule(const std::function<void(void)>& thunk);

/**
 * Blocks and waits until all previously scheduled thunks
 * have been executed in full.
 */
  void wait();

 private:
  
  void dispatcher();
  void worker(size_t workerID);

  typedef struct worker_info {

    thread worker_thread;
    function<void(void)> thunk;
    semaphore worker_sema;
    bool is_available = true;
    bool is_in_use = false; 
  } worker_info;

  thread dt;
  vector<worker_info> wts;
  semaphore all_workers_sema;
  semaphore thunk_sema;
  queue<function<void(void)>> thunks_queue;
  condition_variable_any all_thunks_completed;
  bool isProgramComplete;
  mutex queue_locker;
  mutex remaining_thunks_lock;
  int remaining_thunks;
  mutex worker_field_lock;
  
  
  ThreadPool(const ThreadPool& original) = delete;
  ThreadPool& operator=(const ThreadPool& rhs) = delete;
};

#endif

}
