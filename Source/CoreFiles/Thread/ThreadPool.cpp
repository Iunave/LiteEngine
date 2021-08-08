#include "ThreadPool.hpp"
#include "Memory.hpp"
#include "Log.hpp"

#if DEBUG
FRunnable::FRunnable()
    : TaskName{StrUtil::IntToHex(reinterpret_cast<int64>(this)).Data(), (sizeof(int64) * 2) + 1}
    , TaskProgress{Idle}
{
}
#else
FRunnable::FRunnable()
    : TaskProgress{Idle}
{
}
#endif

#if DEBUG
FRunnable::FRunnable(FString InTaskName)
    : TaskName{InTaskName}
    , TaskProgress{Idle}
{
}
#else
FRunnable::FRunnable(FString InTaskName)
    : TaskProgress{Idle}
{
}
#endif

FRunnable::~FRunnable()
{
    ENSURE(TaskProgress >= 2, "task: {}, is queued or running but was never completed", TaskName);
}

void FRunnable::PreRun()
{
    LOG(LogThread, "started task: {}", TaskName);
    TaskProgress = Running;
}

void FRunnable::PostRun()
{
    LOG(LogThread, "completed task: {}", TaskName);
    TaskProgress = Completed;
}

void FRunnable::WaitForCompletion(const bool bEvenIfNotQueuedOrRunning)  const
{
    const uint8 ProgressValue{static_cast<uint8>(1 + bEvenIfNotQueuedOrRunning)};

    while(EXPECT(TaskProgress <= ProgressValue, false))
    {
        //LOGR(LogThread, "{} is waiting for \"{}\"", StrUtil::IntToHex(pthread_self()), TaskName); todo does not work yet
    }
}

void* ThreadWork(void* Argument)
{
    Thread::FTaskQueue* TaskQueue{static_cast<Thread::FTaskQueue*>(Argument)};

    while(TSharedPtr<FRunnable, ESPMode::Safe> Task{TaskQueue->NextTask()})
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

TSharedPtr<FRunnable, ESPMode::Safe> Thread::FTaskQueue::NextTask()
{
    --QueuedTaskCount;
    TSharedPtr<FRunnable, ESPMode::Safe> TaskToReturn{Move(QueuedTasks[QueuedTaskCount])};
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

Thread::FThreadPool::FThreadPool(const uint64 NumThreadsToStart)
    : NumUsedThreads{NumThreadsToStart}
{
    LOG(LogThread, "creating thread pool with {} threads", NumUsedThreads);

    for(uint64 Index{0}; Index < NumUsedThreads; ++Index)
    {
        WorkerThreads[Index].Create(&::ThreadWork, &TaskQueue);
    }
}

void Thread::FThreadPool::AddTask(TSharedPtr<FRunnable, ESPMode::Safe> NewTask)
{
    TaskQueue.TaskAddMutex.Lock();
#if DEBUG
    if EXPECT(NewTask.IsValid(), true)
    {
        LOG(LogThread, "enqueued task: {}", NewTask->TaskName);
    }
#endif
    --TaskQueue.FreeTaskCount;
    TaskQueue.QueuedTasks[TaskQueue.QueuedTaskCount] = Move(NewTask);
    ++TaskQueue.QueuedTaskCount;

    TaskQueue.TaskAddMutex.Unlock();
}
