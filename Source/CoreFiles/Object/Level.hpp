#pragma once

#include <Interface/IniConfig.hpp>

#include "Definitions.hpp"
#include "Array.hpp"
#include "SmartPointer.hpp"
#include "Object.hpp"

class AActor;

OBJECT_CLASS(OLevel)
class OLevel
{
    OBJECT_BASES()
public:

protected:

    TDynamicArray<TWeakPtr<AActor>> ActorsInLevel;

};

