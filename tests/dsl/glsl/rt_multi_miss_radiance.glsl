#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

layout (location = 0) rayPayloadInEXT vec3 payload0;

void main()
{
    float lvar0;
    lvar0 = 1;
    float lvar1;
    lvar1 = 0.7;
    float lvar2;
    lvar2 = 0.5;
    vec3 lvar3;
    lvar3 = vec3(lvar2, lvar1, lvar0);
    payload0 = lvar3;
}
