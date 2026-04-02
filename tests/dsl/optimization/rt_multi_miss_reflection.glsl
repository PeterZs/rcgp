#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

layout (location = 2) rayPayloadInEXT vec3 payload2;

void main()
{
    payload2 = vec3(0.1, 0.1, 0.1);
}
