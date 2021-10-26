#pragma once

#include <Interface/IniConfig.hpp>

#include "Definitions.hpp"
#include "Array.hpp"
#include "SmartPointer.hpp"
#include "Object.hpp"

class AActor;

OBJECT_CLASS(OLevel)
class OLevel : public OObject
{
    OBJECT_BASES(OObject)
public:

protected:

    TDynamicArray<TWeakPtr<AActor>> ActorsInLevel;

};

