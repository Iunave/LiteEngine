#pragma once

#include "Definitions.hpp"
#include "Object/Object.hpp"
#include "Vector.hpp"
#include "Quaternion.hpp"

OBJECT_CLASS(AActor)
{
    OBJECT_BASES()
public:

    struct FWorldRelationInfo
    {
        FVector Position;
        FQuaternion Rotation;
    };

    INLINE FVector GetActorPosition() const
    {
        return WorldRelationInfo.Position;
    }

    INLINE FQuaternion GetActorRotation() const
    {
        return WorldRelationInfo.Rotation;
    }

private:

    FWorldRelationInfo WorldRelationInfo;

};

