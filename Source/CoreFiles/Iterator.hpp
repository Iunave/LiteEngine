#pragma once

#include "Definitions.hpp"

template<typename ElementType>
class TRangedIterator
{
public:

    explicit constexpr TRangedIterator(ElementType* InPointer)
            : ElementPtr(InPointer)
    {
    }

    constexpr TRangedIterator& operator++()
    {
        ++ElementPtr;
        return *this;
    }

    constexpr TRangedIterator& operator--()
    {
        --ElementPtr;
        return *this;
    }

    constexpr ElementType& operator*()
    {
        return *ElementPtr;
    }

    inline constexpr friend bool operator!=(const TRangedIterator Left, const TRangedIterator Right)
    {
        return Left.ElementPtr != Right.ElementPtr;
    }

private:

    ElementType* ElementPtr;
};
