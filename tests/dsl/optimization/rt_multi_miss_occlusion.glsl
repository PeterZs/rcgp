#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

layout (location = 1) rayPayloadInEXT float payload1;

void main()
{
    payload1 = 1;
}
