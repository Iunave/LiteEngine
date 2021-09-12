#include "Definitions.hpp"
#include "SmartPointer.hpp"
#include "Thread/Thread.hpp"

template<typename FutureType>
class TFuture final
{
public:

    inline TFuture()
        : FuturePointer(nullptr)
    {
    }
/*
    template<typename... VarArgs>
    inline explicit TFuture(VarArgs&&... Args)
        : FuturePointer{MakeShared<FProtectedResult, EThreadMode::Safe>(MoveIfPossible(Args)...)}
    {
        FuturePointer->Mutex.Lock();
    }
*/
    inline TFuture(const TFuture& Other)
    {
        TryUnlock();
        FuturePointer = Other.FuturePointer;
    }

    inline TFuture(TFuture&& Other)
    {
        TryUnlock();
        FuturePointer = Move(Other.FuturePointer);
    }

    inline TFuture& operator=(const TFuture& Other)
    {
        TryUnlock();
        FuturePointer = Other.FuturePointer;
    }

    inline TFuture& operator=(TFuture&& Other)
    {
        TryUnlock();
        FuturePointer = Move(Other.FuturePointer);
    }

    template<typename... VarArgs>
    inline TFuture& Initialize(VarArgs&&... Args)
    {
        //ASSERT(!FuturePointer.IsValid());
        FuturePointer = MakeShared<FProtectedResult, EThreadMode::Safe>(MoveIfPossible(Args)...);
        FuturePointer->Mutex.Lock();
    }

    ~TFuture()
    {
        TryUnlock();
    }

    inline FutureType* operator->()
    {
        return &(FuturePointer->Result);
    }

    inline FutureType& GetResult()
    {
        return FuturePointer->Result;
    }

    void Wait()
    {
        FuturePointer->Mutex.Lock();
        FuturePointer->Mutex.Unlock();
    }

    //future unlocks in the destructor, but if you feel it needs to be unlocked before that do it with this
    inline void TryUnlock()
    {
        if(FuturePointer.IsValid())
        {
            FuturePointer->Mutex.Unlock();
        }
    }

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

    TSharedPtr<FProtectedResult, EThreadMode::Safe> FuturePointer;
};
