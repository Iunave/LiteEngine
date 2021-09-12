#pragma once

#include "Definitions.hpp"
#include "Array.hpp"
#include "Object/Object.hpp"
#include "SmartPointer.hpp"
#include "Level.hpp"

OBJECT_CLASS(OWorld) final : public OObject
{
    OBJECT_BASES(OObject)
public:

private:

    TDynamicArray<TSharedPtr<OLevel>> WorldLevels;

};

