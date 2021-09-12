#pragma once

#include "Definitions.hpp"
#include "TypeTraits.hpp"
#include "Assert.hpp"

template<typename StoredType>
class TOptional final
{
public:

    TOptional()
        : bIsSet(false)
    {
    }

    explicit TOptional(const StoredType& Value)
        : bIsSet(true)
    {
        new(&Storage) StoredType{Value};
    }

    explicit TOptional(StoredType&& Value)
        : bIsSet(true)
    {
        new(&Storage) StoredType{Move(Value)};
    }

    TOptional& operator=(const StoredType& Value)
    {
        bIsSet = true;
        new(&Storage) StoredType{Value};
        return *this;
    }

    TOptional& operator=(StoredType&& Value)
    {
        bIsSet = true;
        new(&Storage) StoredType{Move(Value)};
        return *this;
    }

    constexpr ~TOptional()
    {
        if constexpr(!TypeTrait::IsTriviallyDestructible<StoredType>)
        {
            if(IsSet())
            {
                Storage.Variable::StoredType.~StoredType();
            }
        }
    }

    StoredType& Get()
    {
        ASSERT(IsSet());
        return Storage.Variable;
    }

    INLINE bool IsSet() const
    {
        return bIsSet;
    }

private:

    bool bIsSet;

    union
    {
        StoredType Variable;

    } Storage;

};
