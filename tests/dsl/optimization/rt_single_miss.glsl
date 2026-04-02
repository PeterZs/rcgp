#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

layout (location = 0) rayPayloadInEXT vec3 payload0;

void main()
{
    payload0 = vec3(0, 0, 0);
}
