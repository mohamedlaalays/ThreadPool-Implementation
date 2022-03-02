*Milestone 3**: Implementing the* *ThreadPool* *class*

How does one implement this thread pool thing?  Well, your *ThreadPool* constructor—at least initially—should do the following:

* launch a single dispatcher thread like this (assuming *dt* is a *private* *thread* data member, where *dt* stands for *d*ispatcher *t*hread):

dt = thread(&ThreadPool::dispatcher, this);

* launch a specific number of worker threads like this (assuming *wts* is a *private* *vector<thread>* data member, where *wts* is short for *w*orker thread*s*):

for (size_t workerID = 0; workerID < numThreads; workerID++) {
   wts[workerID] = thread(&ThreadPool::worker, this, workerID);
}

The implementation of *sche**dule* should append the provided thunk to the end of a queue of such functions.  Each time a function is scheduled, the dispatcher thread should be notified.  Once the dispatcher has been notified, *schedule* should return right away so more thunks can be scheduled.

The implementation of the private *dispatcher* method should loop almost interminably, blocking within each iteration until it has confirmation the queue of outstanding functions is nonempty.  It should then wait for a worker thread to become available, select it, mark it as unavailable, dequeue the least recently scheduled function, put a copy of that function in a place where the selected worker (and *only* that worker) can find it, and then signal the worker thread to execute it.

The implementation of the private *worker* method should also loop repeatedly, blocking within each iteration until the dispatcher thread signals it to execute an assigned function (as described above).  Once signaled, the worker should go ahead and invoke the function, wait for it to execute, and then mark itself as available so that it can be discovered and selected again (and again, and again) by the dispatcher.

The implementation of *wait* should block until all previously-scheduled-but-yet-to-be-executed functions have been executed.  Don’t worry if other threads schedule additional thunks while some thread is in *wait*.  If that happens, it just means the waiting thread has to wait a little bit longer.  

The *ThreadPool* destructor should wait until all scheduled functions have executed to completion, somehow inform the dispatcher and worker threads to exit, and then wait for them all to exit.

All of this should feel like a generalization of Assignment 3’s *farm*, and that’s because it is. We’re using threads instead of processes, but the communication patterns between the various players are all the same.  We’re just relying on *thread*s, *mutex*es, conditional variables, and *semaphore*s instead of processes, signals, signal blocks, and *sigwait*.   


*Milestone 4**: Optimizing the* *ThreadPool* *class*

Once you press through your initial *ThreadPool* implementation, I want you to go back and update it to be a little less aggressive about spawning worker threads at construction time.

Consider the scenario where you create a *ThreadPool* of size 32, but the surrounding executable only schedules two or three functions every few seconds.  If the functions are relatively short and execute quickly, the vast majority of the worker threads spawned at construction time will be twiddling their thumbs with nothing to do.  There’s no sense creating worker threads and burdening the thread manager with them until they’re really needed.

To remedy this, you should update your *ThreadPool* implementation to lazily spawn a worker thread only when you need a worker and all existing workers are busy doing something else. Note that a worker thread isn’t truly spawned until a thread with an installed thread routine is moved into the *wts* *vector*.

You should still ensure the number of worker threads never exceeds the thread pool size, but you should only spawn worker threads on an as-needed basis. Once you spawn a worker thread, it can exist forever, even if it never gets used again.
