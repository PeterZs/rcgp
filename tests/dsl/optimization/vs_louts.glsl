#version 460

#extension GL_EXT_scalar_block_layout : require

layout (location = 0) smooth out vec3 lout0;
layout (location = 1) flat out uvec2 lout1;

void main()
{
    lout0 = vec3(vec3(1, 1, 1));
    lout1 = uvec2(uvec2(1, 4));
}
