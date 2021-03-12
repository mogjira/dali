#version 460
#extension GL_GOOGLE_include_directive : enable

#include "common.glsl"

layout(location = 0) in  vec2 inUv;

layout(location = 0) out vec4 outColor;

layout (input_attachment_index = 0, set = 2, binding = 0) uniform subpassInput inputA;

void main()
{
    outColor = subpassLoad(inputA);
}
