#pragma once

#include "Definitions.hpp"

#include <pthread.h>
#include <semaphore.h>

using ThreadType = pthread_t;

enum class EThreadPriority : int32
{
    Highest = 30,
    High = 15,
    Normal = 10,
    Low = 5,
    Lowest = 0
};

static_assert(sizeof(EThreadPriority) == sizeof(sched_param));

namespace Thread
{
    extern const int64 NumRealThreads;

    inline ThreadType MainThreadID{0};
    inline ThreadType AudioThreadID{0};

    class FMutex final
    {
    private:

        struct FAttributeWrapper final
        {
            FAttributeWrapper();
            ~FAttributeWrapper();

            pthread_mutexattr_t MutexAttribute;
        };

        static FAttributeWrapper Attribute;

    public:

        FMutex();
        ~FMutex();

        pthread_mutex_t& GetHandle();

        void Lock();
        bool TryLock();
        void Unlock();

    private:

        pthread_mutex_t MutexHandle;
    };

    class FSharedMutex final
    {
    private:

        struct FAttributeWrapper final
        {
            FAttributeWrapper();
            ~FAttributeWrapper();

            pthread_rwlockattr_t RWLockAttr;
        };

        static FAttributeWrapper Attribute;

    public:

        FSharedMutex();
        ~FSharedMutex();

        pthread_rwlock_t& GetHandle();

        void Lock();
        bool TryLock();

        void SharedLock();
        bool TrySharedLock();

        void Unlock();

    private:

        pthread_rwlock_t RWLockHandle;
    };

    inline FMutex GLogMutex{};

    class FBarrier final
    {
    private:

        struct FAttributeWrapper final
        {
            FAttributeWrapper();
            ~FAttributeWrapper();

            pthread_barrierattr_t BarrierAttribute;
        };

        static FAttributeWrapper Attribute;

    public:

        FBarrier(uint32 ThreadStopCount);
        ~FBarrier();

        void Wait();

    private:

        pthread_barrier_t BarrierHandle;
    };

    template<typename MutexType>
    class FScopedLock final
    {
    public:

        explicit FScopedLock(MutexType& InMutex)
            : Mutex{&InMutex}
        {
            Mutex->Lock();
        }

        ~FScopedLock()
        {
            Mutex->Unlock();
        }

    private:

        MutexType* Mutex;
    };

    template<typename MutexType>
    FScopedLock(MutexType&)->FScopedLock<MutexType>;

    template<typename MutexType>
    class FSharedScopedLock final
    {
    public:

        explicit FSharedScopedLock(MutexType& InMutex)
            : Mutex{&InMutex}
        {
            Mutex->SharedLock();
        }

        ~FSharedScopedLock()
        {
            Mutex->Unlock();
        }

    private:

        MutexType* Mutex;
    };

    template<typename MutexType>
    FSharedScopedLock(MutexType&)->FSharedScopedLock<MutexType>;

    class FSemaphore final
    {
    public:

        FSemaphore(uint32 InitialValue);
        ~FSemaphore();

        int32 Value() const;

        void operator++();
        void operator--();
        auto operator<=>(FSemaphore& Other);

        inline implicit operator int32() const
        {
            return Value();
        }

    private:

        sem_t SemaphoreHandle;
    };

    class FCondition final
    {
    private:

        struct FAttributeWrapper final
        {
            FAttributeWrapper();
            ~FAttributeWrapper();

            pthread_condattr_t CondAttribute;
        };

        static FAttributeWrapper Attribute;

    public:

        FCondition();
        ~FCondition();

        void Signal();

        void Broadcast();

        void Wait(FMutex& Mutex);

    private:

        pthread_cond_t ConditionHandle;
    };

    class FThread
    {
    private:

        struct FAttributeWrapper final
        {
            FAttributeWrapper();

            ~FAttributeWrapper();

            pthread_attr_t ThreadAttribute;
        };

        static FAttributeWrapper Attribute;

    public:

        FThread();

        void Create(void*(*FunctionPtr)(void*), void* Argument);

        void Join();
        bool TryJoin();
        bool TimedJoin(timespec Timeout);

        void Detach();

        void Cancel();

        inline pthread_t GetHandle()
        {
            return ThreadHandle;
        }

    private:

        pthread_t ThreadHandle;
    };

    bool IsInMainThread();
    bool IsInAudioThread();

    void Yield();

}
