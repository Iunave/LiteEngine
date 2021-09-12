#pragma once

#include "Definitions.hpp"
#include "Array.hpp"
#include "SmartPointer.hpp"
#include "Object.hpp"

class AActor;

OBJECT_CLASS(OLevel)
{
    OBJECT_BASES()
public:

protected:

    TDynamicArray<TWeakPtr<AActor>> ActorsInLevel;

};

