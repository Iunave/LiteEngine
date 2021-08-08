#pragma once

#include "Definitions.hpp"
#include "TypeTraits.hpp"

template<typename Type>
concept AtomicConstraints = requires()
{
    TypeTrait::IsPointer<std::decay_t<Type>> || TypeTrait::IsTrivial<std::decay_t<Type>>;
};

namespace Atomic
{

    template<typename T> requires AtomicConstraints<T>
    inline T Add(volatile T* Target, const T Amount)
    {
        return __sync_fetch_and_add(Target, Amount);
    }

    template<typename T> requires AtomicConstraints<T>
    inline T Subtract(volatile T* Target, const T Amount)
    {
        return __sync_fetch_and_sub(Target, Amount);
    }

    template<typename T> requires AtomicConstraints<T>
    inline void Set(volatile T* Target, T Value)
    {
        __atomic_store(Target, &Value, __ATOMIC_SEQ_CST);
    }

    template<typename T> requires AtomicConstraints<T>
    inline T Read(volatile const T* Source)
    {
        T Result;

        __atomic_load(const_cast<volatile T*>(Source), &Result, __ATOMIC_SEQ_CST);

        return Result;
    }

}

template<typename Type> requires AtomicConstraints<Type>
class TAtomic
{
public:

    explicit constexpr TAtomic(Type Value)
        : Variable{Value}
    {
    }

    TAtomic& operator=(Type Value)
    {
        Atomic::Set(&Variable, Value);
        return *this;
    }

    TAtomic operator+(Type Value) const
    {
        return TAtomic{Atomic::Read(&Variable) + Value};
    }

    Type operator++()
    {
        return Atomic::Add(&Variable, static_cast<Type>(1)) + 1;
    }

    Type operator--()
    {
        return Atomic::Subtract(&Variable, static_cast<Type>(1)) - 1;
    }

    TAtomic operator-(Type Value) const
    {
        return TAtomic{Atomic::Read(&Variable) - Value};
    }

    TAtomic& operator+=(Type Value)
    {
        Atomic::Add(&Variable, Value);
        return *this;
    }

    TAtomic& operator-=(Type Value)
    {
        Atomic::Subtract(&Variable, Value);
        return *this;
    }

    bool operator!()
    {
        return !Read();
    }

    Type Read() const
    {
        return Atomic::Read(&Variable);
    }

    implicit operator Type() const
    {
        return Read();
    }

private:

    Type Variable;

};
