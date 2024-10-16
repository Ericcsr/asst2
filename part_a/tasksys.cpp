#include "tasksys.h"
#include <iostream>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    mNumThreads = num_threads;
    mutex_ = new std::mutex();
    //
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::single_thread_spin()
{
    // While loop grab work from tasks
    while (true)
    {
        int i = 0;
        mutex_->lock();
        i = counter_;
        counter_++;
        mutex_->unlock();
        if (i<mNumTasks)
        {
            runnable_->runTask(i, mNumTasks);
        }
        else
        {
            break;
        }
        
    }
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    runnable_ = runnable;
    mNumTasks = num_total_tasks;
    counter_ = 0;

    std::thread* threads = new std::thread[mNumThreads];
    for (int i = 0; i < mNumThreads; i++)
    {
        threads[i] = std::thread(&TaskSystemParallelSpawn::single_thread_spin, this);
    }
    for (int i = 0; i < mNumThreads; i++)
    {
        threads[i].join();
    }
    
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    mNumThreads = num_threads;
    mutex_ = new std::mutex();
    fmutex_ = new std::mutex();
    threads = new std::thread[num_threads];
    for (int i = 0; i < num_threads; i++)
    {
        threads[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::single_thread_spin, this);
    }

}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {

    mutex_->lock();
    quitting = 1;
    mutex_->unlock();
    for (int i = 0; i < mNumThreads; i++)
    {
        threads[i].join();
    }
}

void TaskSystemParallelThreadPoolSpinning::single_thread_spin()
{
    // While loop grab work from tasks
    int i = 0;
    while (true)
    {
        mutex_->lock();
        if(quitting==1)
        {
            mutex_->unlock();
            break;
        }
        if (counter_ < 0)
        {
            mutex_->unlock();
            continue;
        }
        i = counter_;
        counter_--;
        mutex_->unlock();
        //std::cout << i << std::endl;
        runnable_->runTask(i, mNumTasks);
        fmutex_->lock();
        finished_task++;
        fmutex_->unlock();
        
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    mutex_->lock();
    mNumTasks = num_total_tasks;
    runnable_ = runnable;
    counter_ = num_total_tasks-1;
    finished_task = 0;
    mutex_->unlock();

    while (true)
    {
        fmutex_->lock();
        if (finished_task == num_total_tasks)
        {
            fmutex_->unlock();
            break;
        }
        fmutex_->unlock();
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    mNumThreads = num_threads;
    mutex_ = new std::mutex();
    fmutex_ = new std::mutex();
    cv_ = new std::condition_variable();
    threads = new std::thread[num_threads];
    for (int i = 0; i < num_threads; i++)
    {
        threads[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::single_thread_spin, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    mutex_->lock();
    quitting = 1;
    mutex_->unlock();
    cv_->notify_all();
    for (int i = 0; i < mNumThreads; i++)
    {
        threads[i].join();
    }

}

void TaskSystemParallelThreadPoolSleeping::single_thread_spin()
{
    // While loop grab work from tasks
    int i = 0;
    while (true)
    {
        mutex_->lock();
        if(quitting==1)
        {
            mutex_->unlock();
            break;
        }
        if (counter_ < 0)
        {
            mutex_->unlock();
            std::unique_lock<std::mutex> lk(*mutex_);
            cv_->wait(lk);
            continue;
        }
        i = counter_;
        counter_--;
        mutex_->unlock();
        // std::cout << i << std::endl;
        runnable_->runTask(i, mNumTasks);
        fmutex_->lock();
        finished_task++;
        fmutex_->unlock();
    }
        
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    mutex_->lock();
    mNumTasks = num_total_tasks;
    runnable_ = runnable;
    counter_ = num_total_tasks-1;
    finished_task = 0;
    
    mutex_->unlock();
    cv_->notify_all();

    while (true)
    {
        fmutex_->lock();
        if (finished_task == num_total_tasks)
        {
            fmutex_->unlock();
            break;
        }
        fmutex_->unlock();
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
