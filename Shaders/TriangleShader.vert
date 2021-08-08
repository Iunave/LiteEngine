#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 FragColor;

vec2 VertexPositions[6] = vec2[6]
(
    vec2(0.0, 0.5),
    vec2(0.5, -0.5),
    vec2(-0.5, -0.5),
    vec2(0.0, -0.825),
    vec2(-0.5, 0.125),
    vec2(0.5, 0.125)
);

vec3 Colors[6] = vec3[6]
(
    vec3(1.0, 0.2, 0.2),
    vec3(1.0, 0.2, 0.2),
    vec3(1.0, 0.2, 0.2),
    vec3(1.0, 0.2, 0.2),
    vec3(1.0, 0.2, 0.2),
    vec3(1.0, 0.2, 0.2)
);

void main()
{
    gl_Position = vec4(VertexPositions[gl_VertexIndex], 0.0, 1.0);

    FragColor = Colors[gl_VertexIndex];
}
