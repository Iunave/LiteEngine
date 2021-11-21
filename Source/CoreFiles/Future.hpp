#include "Definitions.hpp"
#include "SmartPointer.hpp"
#include "Thread/Thread.hpp"

template<typename FutureType>
class TFuture final
{
private:

    struct FProtectedResult
    {
        template<typename... VarArgs>
        explicit FProtectedResult(VarArgs&&... Args)
            : Result{MoveIfPossible(Args)...}
        {
        }

        Thread::FMutex Mutex;
        FutureType Result;
    };

public:

    TFuture()
        : FuturePointer{nullptr}
    {
    }

    template<typename... VarArgs>
    TFuture(VarArgs&&... Args)
        : FuturePointer{MoveIfPossible(Args)...}
    {
    }

    TFuture(TFuture&& Other)
        : FuturePointer{Move(Other.FuturePointer)}
    {
    }

    TFuture(const TFuture& Other)
        : FuturePointer{Other.FuturePointer}
    {
    }

    TFuture& operator=(const TFuture& Other)
    {
        FuturePointer = Other.FuturePointer;
    }

    TFuture& operator=(TFuture&& Other)
    {
        FuturePointer = Move(Other.FuturePointer);
    }

    ~TFuture()
    {
        TryUnlock();
    }

    FutureType* operator->()
    {
        Thread::FScopedLock Lock{FuturePointer->Mutex};
        return &FuturePointer->Result;
    }

    FutureType& operator*()
    {
        Thread::FScopedLock Lock{FuturePointer->Mutex};
        return FuturePointer->Result;
    }

    FutureType& GetResult()
    {
        return FuturePointer->Result;
    }

    void Wait()
    {
        FuturePointer->Mutex.Lock();
        FuturePointer->Mutex.Unlock();
    }

    //future unlocks in the destructor, but if you feel it needs to be unlocked before that do it with this
    void TryUnlock()
    {
        if(FuturePointer.IsValid())
        {
            FuturePointer->Mutex.Unlock();
        }
    }

private:

    TSharedPtr<FProtectedResult, EThreadMode::Safe> FuturePointer;
};
