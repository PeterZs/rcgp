#version 460

#extension GL_EXT_scalar_block_layout : require

layout (location = 0) in vec3 lin0;
layout (location = 1) in vec3 lin1;
layout (location = 2) in vec2 lin2;

layout (location = 0) smooth out vec3 lout0;
layout (location = 1) smooth out vec3 lout1;
layout (location = 2) smooth out vec2 lout2;

void main()
{
    lout0 = vec3(vec3(lin0));
    lout1 = vec3(vec3(lin1));
    lout2 = vec2(vec2(lin2));
}
