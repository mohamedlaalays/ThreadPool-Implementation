# ThreadPool Class Implementation

The program has three public methods:

## ThreadPool(size_t numThreads)
A constructor that initializes an instance of ThreadPool and lazily spawns numThreads threads.

## schedule(const std::function<void(void)>& thunk)
The method executes the function provided by the client

## wait()
When the client invokes this method, all the running threads are waited to finish. 
