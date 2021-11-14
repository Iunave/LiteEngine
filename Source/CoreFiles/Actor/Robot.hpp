#pragma once

#include "Definitions.hpp"
#include "Array.hpp"
#include "SmartPointer.hpp"
#include "Actor.hpp"
#include "BuildComponent/BuildComponent.hpp"

OBJECT_CLASS(ARobot)
class ARobot : public AActor
{
    OBJECT_BASES(AActor)
    friend class ABuildComponent;
public:

    void AddComponent(TSharedPtr<ABuildComponent> NewComponent);

protected:

    TDynamicArray<ABuildComponent> Components;
};
