#include "ThreadPool.hpp"
#include "Memory.hpp"
#include "Log.hpp"

ORunnable::ORunnable()
    : TaskProgress{Idle}
{
}

ORunnable::~ORunnable()
{
    ENSURE(TaskProgress != Queued || TaskProgress != Running, "task: {}, is queued or running but was never completed", GetClassName());
}

void ORunnable::StartAsyncTask()
{
    Thread::AsyncTask(this);
}

void ORunnable::PreRun()
{
    LOG(LogThread, "started task: {}", GetClassName());
    TaskProgress = Running;
}

void ORunnable::PostRun()
{
    LOG(LogThread, "completed task: {}", GetClassName());
    TaskProgress = Completed;
}

void ORunnable::WaitForCompletion() const
{
    while(TaskProgress != Completed)
    {
        pthread_yield();
    }
}

void* ThreadWork(void* Argument)
{
    Thread::FTaskQueue* TaskQueue{static_cast<Thread::FTaskQueue*>(Argument)};

    while(ORunnable* Task{TaskQueue->NextTask()})
    {
        Task->PreRun();
        Task->Run();
        Task->PostRun();
    }

    pthread_exit(nullptr);
}

Thread::FTaskQueue::FTaskQueue()
    : QueuedTaskCount{0}
    , FreeTaskCount{QueuePoolSize}
    , QueuedTasks{}
{
}

Thread::FTaskQueue::~FTaskQueue()
{
}

ORunnable* Thread::FTaskQueue::NextTask()
{
    --QueuedTaskCount;
    ORunnable* TaskToReturn{QueuedTasks[QueuedTaskCount.Value()]};
    ++FreeTaskCount;

    return TaskToReturn;
}

Thread::FThreadPool::~FThreadPool()
{
    for(uint64 Index{0}; Index < NumUsedThreads; ++Index)
    {
        AddTask(nullptr);
    }

    for(uint64 Index{0}; Index < NumUsedThreads; ++Index)
    {
        WorkerThreads[Index].Join();
    }
}

Thread::FThreadPool::FThreadPool()
    : NumUsedThreads{static_cast<uint64>(::sysconf(_SC_NPROCESSORS_CONF))}
{
    LOG(LogThread, "creating thread pool with {} threads", NumUsedThreads);

    for(uint64 Index{0}; Index < NumUsedThreads; ++Index)
    {
        WorkerThreads[Index].Create(&::ThreadWork, &TaskQueue);
    }
}

void Thread::FThreadPool::AddTask(ORunnable* NewTask)
{
    TaskQueue.TaskAddMutex.Lock();

    if EXPECT(NewTask != nullptr, true)
    {
        LOG(LogThread, "enqueued task: {}", NewTask->GetClassName());
    }

    --TaskQueue.FreeTaskCount;
    TaskQueue.QueuedTasks[TaskQueue.QueuedTaskCount.Value()] = NewTask;
    ++TaskQueue.QueuedTaskCount;

    TaskQueue.TaskAddMutex.Unlock();
}
