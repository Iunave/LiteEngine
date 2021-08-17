#pragma once

#include "Definitions.hpp"
#include "Thread/Thread.hpp"
#include "SmartPointer.hpp"
#include "String.hpp"
#include "Array.hpp"

class FRunnable
{
    friend void* ThreadWork(void*);
public:

    enum ETaskProgress : uint8
    {
        Queued = 0,
        Running = 1,
        Idle = 2,
        Completed = 3
    };

    FRunnable(FString InTaskName);

    FRunnable();

    virtual ~FRunnable();

    virtual void Run() = 0;

    void WaitForCompletion(const bool bEvenIfNotQueuedOrRunning = true) const;

#if DEBUG
    const FString TaskName;
#endif

    INLINE ETaskProgress GetTaskProgress() const
    {
        return TaskProgress.Read();
    }

private:

    void PreRun();
    void PostRun();

    TAtomic<ETaskProgress> TaskProgress;
};

namespace Thread
{

    class FTaskQueue final
    {
        friend class FThreadPool;
        friend void* ::ThreadWork(void*);
    public:

        static constexpr int64 QueuePoolSize{256};

        FTaskQueue();

        ~FTaskQueue();

        TSharedPtr<FRunnable, ESPMode::Safe> NextTask();

    private:

        Thread::FMutex TaskAddMutex;

        Thread::FSemaphore QueuedTaskCount;
        Thread::FSemaphore FreeTaskCount;

        TStaticArray<TSharedPtr<FRunnable, ESPMode::Safe>, QueuePoolSize> QueuedTasks;
    };

    class FThreadPool final : public FSingleton<FThreadPool>
    {
    public:

        FThreadPool();

        ~FThreadPool();

        void AddTask(TSharedPtr<FRunnable, ESPMode::Safe> NewTask);

    private:

        //allocate for a maximum of 64 threads, we probably will use less
        TStaticArray<Thread::FThread, 64> WorkerThreads;
        const uint64 NumUsedThreads;

        FTaskQueue TaskQueue;
    };

    template<typename RunnableClass> requires(TypeTrait::IsBaseOf<FRunnable, RunnableClass>)
    INLINE void AsyncTask(TSharedPtr<RunnableClass, ESPMode::Safe> NewTask)
    {
        ASSERT(NewTask.IsValid());
        FThreadPool::Instance().AddTask(Move(NewTask));
    }
}

