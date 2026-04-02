#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

layout (location = 1) rayPayloadInEXT float payload1;

void main()
{
    float lvar0;
    lvar0 = 1;
    payload1 = lvar0;
}
