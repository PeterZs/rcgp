#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

layout (location = 0) rayPayloadInEXT vec3 payload0;

void main()
{
    float lvar0;
    lvar0 = 0;
    vec3 lvar1;
    lvar1 = vec3(lvar0, lvar0, lvar0);
    payload0 = lvar1;
}
