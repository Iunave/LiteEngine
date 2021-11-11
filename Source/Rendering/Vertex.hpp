#pragma once

#include "Definitions.hpp"
#include "Vector.hpp"
#include "Color.hpp"
#include "Array.hpp"

#include <vulkan/vulkan.hpp>

class FVertex
{
public:

    static const constinit Vk::VertexInputBindingDescription BindingDescription;
    static const constinit TStaticArray<Vk::VertexInputAttributeDescription, 2> AttributeDescriptions;

    FVector2D Position;
    RGBA32F Color;
};

inline constexpr TStaticArray<FVertex, 3> Vertices_Test
{
    FVertex{FVector2D{0.0f, -0.5f}, RGBA32F{1.0f, 0.0f, 1.0f, 1.0f}},
    FVertex{FVector2D{0.5f, 0.5f}, RGBA32F{0.0f, 1.0f, 0.0f, 1.0f}},
    FVertex{FVector2D{-0.5f, 0.5f}, RGBA32F{0.0f, 0.0f, 1.0f, 1.0f}}
};
