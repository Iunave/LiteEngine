#include "Thread.hpp"
#include "Log.hpp"
#include "String.hpp"
#include "Function.hpp"

#include <unistd.h>

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

#endif //DEBUG

namespace Thread
{
    FMutex::FAttributeWrapper FMutex::Attribute{};
    FBarrier::FAttributeWrapper FBarrier::Attribute{};
    FCondition::FAttributeWrapper FCondition::Attribute{};
    FThread::FAttributeWrapper FThread::Attribute{};

    const int64 NumRealThreads{::sysconf(_SC_NPROCESSORS_CONF)};

    bool IsInMainThread()
    {
        return pthread_self() == MainThreadId;
    }

    FMutex::FAttributeWrapper::FAttributeWrapper()
    {
        pthread_mutexattr_t MutexAttributes;
        pthread_mutexattr_init(&MutexAttributes);
        pthread_mutexattr_settype(&MutexAttributes, PTHREAD_MUTEX_DEFAULT);
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
        pthread_mutex_lock(&MutexHandle);
    }

    bool FMutex::TryLock()
    {
        return pthread_mutex_trylock(&MutexHandle) == 0;
    }

    void FMutex::Unlock()
    {
        pthread_mutex_unlock(&MutexHandle);
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
        : ThreadHandle(UINT32_MAX)
    {
    }

    void FThread::Create(void*(*FunctionPtr)(void*), void* Argument)
    {
        const int32 CreateResult{pthread_create(&ThreadHandle, &Attribute.ThreadAttribute, FunctionPtr, Argument)};
        LOG(LogThread, "{} started [{}]", StrUtil::IntToHex(ThreadHandle), CreateResultToString(CreateResult));
    }

    void FThread::Join()
    {
        const int32 JoinResult{pthread_join(ThreadHandle, nullptr)};
        LOG(LogThread, "{} joined with {} [{}]", StrUtil::IntToHex(pthread_self()), StrUtil::IntToHex(ThreadHandle), JoinResultToString(JoinResult));
    }

    bool FThread::TryJoin()
    {
        const int32 JoinResult{pthread_tryjoin_np(ThreadHandle, nullptr)};
        LOG(LogThread, "{} tried to join with {} [{}]", StrUtil::IntToHex(pthread_self()), StrUtil::IntToHex(ThreadHandle), TryJoinResultToString(JoinResult));
        return JoinResult == 0;
    }

    bool FThread::TimedJoin(timespec Timeout)
    {
        const int32 JoinResult{pthread_timedjoin_np(ThreadHandle, nullptr, &Timeout)};
        LOG(LogThread, "{} tried to join with {} [{}]", StrUtil::IntToHex(pthread_self()), StrUtil::IntToHex(ThreadHandle), TryJoinResultToString(JoinResult));
        return JoinResult == 0;
    }

    void FThread::Detach()
    {
        const int32 DetachResult{pthread_detach(ThreadHandle)};
        LOG(LogThread, "{} has detached [{}]", StrUtil::IntToHex(ThreadHandle), DetachResultToString(DetachResult));
    }
}
