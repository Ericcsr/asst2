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
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
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
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */
void TaskSystemParallelThreadPoolSleeping::single_thread_spin()
{
    // While loop grab work from readyQueue
    while (true)
    {
        readyTasks_ -> lock();
        if(readyTasks.empty()){
            if (finishedTask.load() >= mTargetTasks) {
                readyTasks_ -> unlock();
                cv2_-> notify_one();
                return;
            }
            readyTasks_ -> unlock();
            continue;
        }

        int id = readyTasks.front();
        // mTaskLock_[id] -> lock();
        int i = mFinishedTask[id] + mRunningTask[id];

        // The last task for taskId id;
        if(mNumTasks[id] == i + 1){
            readyTasks.pop();
        }
        mRunningTask[id] ++;
        readyTasks_ -> unlock();
        //mTaskLock_[id] -> unlock();

        auto runnable = mRunnable[id];
        int numTasks = mNumTasks[id];
        runnable->runTask(i, numTasks);

        //mTaskLock_[id] -> lock();
        readyTasks_ -> lock();
        mFinishedTask[id] ++;
        mRunningTask[id] --;
        if(mFinishedTask[id] == numTasks){
            //std::cout << "task: " << id << " done " << mTotalTasks << " " << finishedTask << std::endl;
            for(auto depId: mSupportTask[id]){
                //mTaskLock_[depId] -> lock();
                mBlockNum[depId] --;
                if(mBlockNum[depId] == 0){
                    //readyTasks_ -> lock();
                    readyTasks.push(depId);
                    //readyTasks_ -> unlock();
                }
                //mTaskLock_[depId] -> unlock();
            }
            int current = ++finishedTask;
            if (current >= mTargetTasks) {
                //mTaskLock_[id] -> unlock();
                cv2_-> notify_one();
                readyTasks_ -> unlock();
                return;
            }
        }
        //mTaskLock_[id] -> unlock();
        readyTasks_ -> unlock();
    }  
}


const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).

    readyTasks_ = new std::mutex();
    mTaskIdCnt_ = new std::mutex();
    fmutex_ = new std::mutex();
    cv2_ = new std::condition_variable();

    mNumThreads = num_threads;
    mFinishedTask.resize(MaxTaskNum);
    mRunningTask.resize(MaxTaskNum);
    mTaskLock_.resize(MaxTaskNum);
    mNumTasks.resize(MaxTaskNum);
    mBlockNum.resize(MaxTaskNum);
    mRunnable.resize(MaxTaskNum);
    mSupportTask.resize(MaxTaskNum);
    mTaskIdCnt = 0;
    mTargetTasks = MaxTaskNum * 2;
    finishedTask = 0;
    
    threads = new std::thread[num_threads];
    for (int i = 0; i < num_threads; i++)
    {
        threads[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::single_thread_spin, this);
    }
    readyTasks_ -> unlock();
    mTaskIdCnt_ -> unlock();

}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    finishedTask = 0;
    mTotalTasks = 0;
    mTaskIdCnt = 0;
    mTargetTasks = -1;
    for (int i = 0; i < mNumThreads; i++)
    {
        threads[i].join();
    }
    mTargetTasks = MaxTaskNum * 2;
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    runAsyncWithDeps(runnable, num_total_tasks, std::vector<TaskID>());
    sync();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //
    int taskId;
    mTotalTasks ++;
    taskId = mTaskIdCnt;
    mTaskIdCnt ++;

    mTaskLock_[taskId] = new std::mutex();
    mNumTasks[taskId] = num_total_tasks;
    mBlockNum[taskId] = 0;
    mFinishedTask[taskId] = 0;
    mRunnable[taskId] = runnable;
    mSupportTask[taskId].clear();
    
    int blockNum = 0;
    readyTasks_-> lock();
    // mTaskLock_[taskId] -> lock();
    for(auto dependId: deps){
        // mTaskLock_[dependId] -> lock();
        if(mFinishedTask[dependId] < mNumTasks[dependId]){
            blockNum++;
            mSupportTask[dependId].push_back(taskId);
        }
    }
    if(!blockNum){
        
        readyTasks.push(taskId);
        
    }

    
    mBlockNum[taskId] = blockNum;
    

    // for(auto dependId:deps){
    //     mTaskLock_[dependId] -> unlock();
    // }
    //mTaskLock_[taskId] -> unlock();
    readyTasks_->unlock();
    return taskId;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    mTargetTasks = mTotalTasks;
    if(finishedTask >= mTargetTasks){
    }
    else{
        std::unique_lock<std::mutex> lk(*fmutex_);
        cv2_ -> wait(lk, [this]{ return finishedTask.load() >= mTargetTasks; });
    }
    
    finishedTask = 0;
    mTotalTasks = 0;
    mTaskIdCnt = 0;
    mTargetTasks = MaxTaskNum * 2;
}
