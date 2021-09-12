#pragma once

#include "Definitions.hpp"
#include "Thread/Thread.hpp"
#include "String.hpp"
#include "Array.hpp"
#include "Object/Object.hpp"

namespace Thread
{
    class FTaskQueue;
    class FThreadPool;
}

OBJECT_CLASS(ORunnable)
{
    OBJECT_BASES()

    friend void* ThreadWork(void*);
    friend class Thread::FThreadPool;
public:

    enum ETaskProgress : uint8
    {
        Queued = 0,
        Running = 1,
        Idle = 2,
        Completed = 3
    };

    ORunnable();

    virtual ~ORunnable();

    virtual void Run() = 0;

    void WaitForCompletion(bool bEvenIfNotInQueue = true) const;

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
        friend void* ::ThreadWork(void*);
        friend class FThreadPool;
    public:

        static constexpr int64 QueuePoolSize{256};

        FTaskQueue();

        ~FTaskQueue();

        ORunnable* NextTask();

    private:

        Thread::FMutex TaskAddMutex;

        Thread::FSemaphore QueuedTaskCount;
        Thread::FSemaphore FreeTaskCount;

        TStaticArray<ORunnable*, QueuePoolSize> QueuedTasks;
    };

    class FThreadPool final : public FSingleton<FThreadPool>
    {
    public:

        FThreadPool();

        ~FThreadPool();

        void AddTask(ORunnable* NewTask);

    private:

        //allocate for a maximum of 64 threads, we probably will use less
        TStaticArray<Thread::FThread, 64> WorkerThreads;
        const uint64 NumUsedThreads;

        FTaskQueue TaskQueue;
    };

    INLINE void AsyncTask(ORunnable* NewTask)
    {
        ASSERT(NewTask != nullptr);
        FThreadPool::Instance().AddTask(NewTask);
    }
}

