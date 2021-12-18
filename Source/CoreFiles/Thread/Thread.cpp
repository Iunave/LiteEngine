#include "Thread.hpp"
#include "Log.hpp"

#if DEBUG

inline const char8* CreateResultToString(int32 Value)
{
    switch(Value)
    {
        case 0: return "success";
        case EAGAIN: return "ERROR: insufficient resources or system does not allow this many threads";
        case EINVAL: return "ERROR: invalid settings in attr";
        case EPERM: return "ERROR: No permission to set the scheduling policy and parameters specified in attr";
        default: return "ERROR: unknown";
    }
}

inline const char8* JoinResultToString(int32 Value)
{
    switch(Value)
    {
        case 0: return "success";
        case EDEADLOCK: return "ERROR: detected deadlock";
        case EINVAL: return "ERROR: thread is not joinable or another thread is already waiting to join";
        case ESRCH: return "ERROR: thread not found";
        default: return "ERROR: unknown";
    }
}

inline const char8* TryJoinResultToString(int32 Value)
{
    switch(Value)
    {
        case 0: return "success";
        case EBUSY: return "fail: thread is still running";
        case ETIMEDOUT: return "fail: time out";
        case EINVAL: return "ERROR: invalid timeout value";
        default: return "ERROR: unknown";
    }
}

inline const char8* DetachResultToString(int32 Value)
{
    switch(Value)
    {
        case 0: return "success";
        case EINVAL: return "ERROR: thread is already detached";
        case ESRCH: return "ERROR: thread not found";
        default: return "ERROR: unknown";
    }
}

inline const char8* LockResultToString(int32 Value)
{
    switch(Value)
    {
        case 0: return "success";
        case EAGAIN: return "ERROR: the  mutex  could  not be acquired because the maximum number of recursive locks for mutex has been exceeded";
        case EINVAL: return "ERROR: the mutex was created with the  protocol  attribute  having  the value  PTHREAD_PRIO_PROTECT and the calling thread's priority is higher than the mutex's current priority ceiling";
        case ENOTRECOVERABLE: return "ERROR: the state protected by the mutex is not recoverable";
        case EOWNERDEAD: return "ERROR: the mutex is a robust mutex and the process containing the previous owning thread terminated while holding the mutex lock. The mutex lock shall be acquired by the calling thread and it is  up to the new owner to make the state consistent";
        case EDEADLK: return "ERROR: the  mutex  type  is PTHREAD_MUTEX_ERRORCHECK and the current thread already owns the mutex";
        case EBUSY: return "ERROR: the mutex could not be acquired because it was already locked";
        case EPERM: return "ERROR: the mutex type is PTHREAD_MUTEX_ERRORCHECK or PTHREAD_MUTEX_RECURSIVE, or the mutex is a robust mutex, and the current thread does not own the mutex";
        default: return "ERROR: unknown";
    }
}

inline const char8* UnlockResultToString(int32 Value)
{
    switch(Value)
    {
        case 0: return "success";
        case EINVAL: return "ERROR: the value specified by mutex is not valid";
        case EPERM: return "ERROR: the current thread does not own the mutex";
        default: return "ERROR: unknown";
    }
}

inline const char8* ReadLockResultToString(int32 Value)
{
    switch(Value)
    {
        case 0: return "success";
        case EAGAIN: return "ERROR: The read lock could not be acquired because the  maximum  number of read locks for rwlock has been exceeded.";
        case EDEADLOCK: return "deadlock condition was detected or the current thread already owns the read-write lock for writing.";
        default: return "ERROR: unknown";
    }
}

#endif //DEBUG

namespace Thread
{
    FMutex::FAttributeWrapper FMutex::Attribute{};
    FSharedMutex::FAttributeWrapper FSharedMutex::Attribute{};
    FBarrier::FAttributeWrapper FBarrier::Attribute{};
    FCondition::FAttributeWrapper FCondition::Attribute{};
    FThread::FAttributeWrapper FThread::Attribute{};

    #if defined(__unix__)
    const int64 NumRealThreads{::sysconf(_SC_NPROCESSORS_CONF)};
    #elif defined(_WIN32)
    const int64 NumRealThreads{???}; //todo
    #endif

    FMutex::FAttributeWrapper::FAttributeWrapper()
    {
        pthread_mutexattr_init(&MutexAttribute);
        pthread_mutexattr_setprotocol(&MutexAttribute, PTHREAD_PRIO_INHERIT);
        pthread_mutexattr_setpshared(&MutexAttribute, PTHREAD_PROCESS_PRIVATE);
        pthread_mutexattr_setrobust(&MutexAttribute, PTHREAD_MUTEX_STALLED);
#if DEBUG
        pthread_mutexattr_settype(&MutexAttribute, PTHREAD_MUTEX_ERRORCHECK_NP);
#else
        pthread_mutexattr_settype(&MutexAttribute, PTHREAD_MUTEX_FAST_NP);
#endif
    }

    FMutex::FAttributeWrapper::~FAttributeWrapper()
    {
        pthread_mutexattr_destroy(&MutexAttribute);
    }

    FMutex::FMutex()
    {
        pthread_mutex_init(&MutexHandle, &Attribute.MutexAttribute);
    }

    FMutex::~FMutex()
    {
        pthread_mutex_destroy(&MutexHandle);
    }

    pthread_mutex_t& FMutex::GetHandle()
    {
        return MutexHandle;
    }

    void FMutex::Lock()
    {
        int32 Result{pthread_mutex_lock(&MutexHandle)};
        ENSURE(Result == 0, "{}", LockResultToString(Result));
    }

    bool FMutex::TryLock()
    {
        int32 Result{pthread_mutex_trylock(&MutexHandle)};
        ENSURE(Result == 0 || Result == EBUSY, "{}", LockResultToString(Result));
        return Result == 0;
    }

    void FMutex::Unlock()
    {
        int32 Result{pthread_mutex_unlock(&MutexHandle)};
        ENSURE(Result == 0, "{}", UnlockResultToString(Result));
    }

    FSharedMutex::FAttributeWrapper::FAttributeWrapper()
    {
        pthread_rwlockattr_init(&RWLockAttr);
        pthread_rwlockattr_setkind_np(&RWLockAttr, PTHREAD_RWLOCK_PREFER_WRITER_NP);
    }

    FSharedMutex::FAttributeWrapper::~FAttributeWrapper()
    {
        pthread_rwlockattr_destroy(&RWLockAttr);
    }

    FSharedMutex::FSharedMutex()
    {
        pthread_rwlock_init(&RWLockHandle, &Attribute.RWLockAttr);
    }

    FSharedMutex::~FSharedMutex()
    {
        pthread_rwlock_destroy(&RWLockHandle);
    }

    pthread_rwlock_t& FSharedMutex::GetHandle()
    {
        return RWLockHandle;
    }

    void FSharedMutex::Lock()
    {
        int32 Result{pthread_rwlock_wrlock(&RWLockHandle)};
        ASSERT(Result != EDEADLOCK, "deadlock detected");
    }

    bool FSharedMutex::TryLock()
    {
        return pthread_rwlock_trywrlock(&RWLockHandle) == 0;
    }

    void FSharedMutex::SharedLock()
    {
        int32 Result{pthread_rwlock_rdlock(&RWLockHandle)};
        ASSERT(Result == 0, "{}", ReadLockResultToString(Result));
    }

    bool FSharedMutex::TrySharedLock()
    {
        return pthread_rwlock_tryrdlock(&RWLockHandle) == 0;
    }

    void FSharedMutex::Unlock()
    {
        pthread_rwlock_unlock(&RWLockHandle);
    }

    FBarrier::FAttributeWrapper::FAttributeWrapper()
    {
        pthread_barrierattr_init(&BarrierAttribute);
        pthread_barrierattr_setpshared(&BarrierAttribute, PTHREAD_PROCESS_SHARED);
    }

    FBarrier::FAttributeWrapper::~FAttributeWrapper()
    {
        pthread_barrierattr_destroy(&BarrierAttribute);
    }

    FBarrier::FBarrier(uint32 ThreadStopCount)
    {
        pthread_barrier_init(&BarrierHandle, &Attribute.BarrierAttribute, ThreadStopCount);
    }

    FBarrier::~FBarrier()
    {
        pthread_barrier_destroy(&BarrierHandle);
    }

    void FBarrier::Wait()
    {
        pthread_barrier_wait(&BarrierHandle);
    }

    FSemaphore::FSemaphore(uint32 InitialValue)
    {
        sem_init(&SemaphoreHandle, 0, InitialValue);
    }

    FSemaphore::~FSemaphore()
    {
        sem_destroy(&SemaphoreHandle);
    }

    int32 FSemaphore::Value() const
    {
        int32 Value;
        sem_getvalue(const_cast<sem_t*>(&SemaphoreHandle), &Value);
        return Value;
    }

    void FSemaphore::operator++()
    {
        sem_post(&SemaphoreHandle);
    }

    void FSemaphore::operator--()
    {
        sem_wait(&SemaphoreHandle);
    }

    auto FSemaphore::operator<=>(FSemaphore& Other)
    {
        return Value() <=> Other.Value();
    }

    FCondition::FAttributeWrapper::FAttributeWrapper()
    {
        pthread_condattr_init(&CondAttribute);
    }

    FCondition::FAttributeWrapper::~FAttributeWrapper()
    {
        pthread_condattr_destroy(&CondAttribute);
    }

    FCondition::FCondition()
    {
        pthread_cond_init(&ConditionHandle, &Attribute.CondAttribute);
    }

    FCondition::~FCondition()
    {
        pthread_cond_destroy(&ConditionHandle);
    }

    void FCondition::Signal()
    {
        pthread_cond_signal(&ConditionHandle);
    }

    void FCondition::Wait(FMutex& Mutex)
    {
        pthread_cond_wait(&ConditionHandle, &Mutex.GetHandle());
    }

    void FCondition::Broadcast()
    {
        pthread_cond_broadcast(&ConditionHandle);
    }

    FThread::FAttributeWrapper::FAttributeWrapper()
    {
        pthread_attr_init(&ThreadAttribute);
        pthread_attr_setdetachstate(&ThreadAttribute, PTHREAD_CREATE_JOINABLE);
    }

    FThread::FAttributeWrapper::~FAttributeWrapper()
    {
        pthread_attr_destroy(&ThreadAttribute);
    }

    FThread::FThread()
        : ThreadHandle{UINT32_MAX}
    {
    }

    void FThread::Create(void*(*FunctionPtr)(void*), void* Argument)
    {
        const int32 CreateResult{pthread_create(&ThreadHandle, &Attribute.ThreadAttribute, FunctionPtr, Argument)};
        LOG(LogThread, "{} started [{}]", (void*)ThreadHandle, CreateResultToString(CreateResult));
    }

    void FThread::Join()
    {
        const int32 JoinResult{pthread_join(ThreadHandle, nullptr)};
        LOG(LogThread, "{} joined with {} [{}]", (void*)pthread_self(), (void*)ThreadHandle, JoinResultToString(JoinResult));
    }

    bool FThread::TryJoin()
    {
        const int32 JoinResult{pthread_tryjoin_np(ThreadHandle, nullptr)};
        LOG(LogThread, "{} tried to join with {} [{}]", (void*)pthread_self(), (void*)ThreadHandle, TryJoinResultToString(JoinResult));
        return JoinResult == 0;
    }

    bool FThread::TimedJoin(timespec Timeout)
    {
        const int32 JoinResult{pthread_timedjoin_np(ThreadHandle, nullptr, &Timeout)};
        LOG(LogThread, "{} tried to join with {} [{}]", (void*)pthread_self(), (void*)ThreadHandle, TryJoinResultToString(JoinResult));
        return JoinResult == 0;
    }

    void FThread::Detach()
    {
        const int32 DetachResult{pthread_detach(ThreadHandle)};
        LOG(LogThread, "{} has detached [{}]", (void*)ThreadHandle, DetachResultToString(DetachResult));
    }

    void FThread::Cancel()
    {
        pthread_cancel(ThreadHandle);
    }

    bool IsInMainThread()
    {
        ASSERT(MainThreadID != 0, "main thread ID is not initialized");
        return pthread_self() == MainThreadID;
    }

    bool IsInAudioThread()
    {
        ASSERT(AudioThreadID != 0, "audio thread ID is not initialized");
        return pthread_self() == AudioThreadID;
    }

    void Yield()
    {
        sched_yield();
    }
}
