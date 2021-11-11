#include "Rendering/Vertex.hpp"

const constinit Vk::VertexInputBindingDescription FVertex::BindingDescription
{
    []()
    {
        Vk::VertexInputBindingDescription BindingDescription{};
        BindingDescription.binding = 0;
        BindingDescription.stride = sizeof(FVertex);
        BindingDescription.inputRate = Vk::VertexInputRate::eVertex;

        return BindingDescription;
    }()
};

const constinit TStaticArray<Vk::VertexInputAttributeDescription, 2> FVertex::AttributeDescriptions
{
    []()
    {
        TStaticArray<Vk::VertexInputAttributeDescription, 2> Descriptions{};
        Descriptions[0].binding = 0;
        Descriptions[0].location = 0;
        Descriptions[0].offset = offsetof(FVertex, Position);
        Descriptions[0].format = Vk::Format::eR32G32Sfloat;

        Descriptions[1].binding = 0;
        Descriptions[1].location = 1;
        Descriptions[1].offset = offsetof(FVertex, Color);
        Descriptions[1].format = Vk::Format::eR32G32B32A32Sfloat;

        return Descriptions;
    }()
};
