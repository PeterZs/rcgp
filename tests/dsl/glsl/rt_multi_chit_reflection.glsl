#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

hitAttributeEXT vec2 hit_attribute;

layout (location = 2) rayPayloadInEXT vec3 payload2;

void main()
{
    vec2 lvar0;
    lvar0 = vec2(hit_attribute);
    float lvar1;
    lvar1 = 1;
    float lvar2;
    lvar2 = lvar0.y;
    float lvar3;
    lvar3 = lvar0.x;
    vec3 lvar4;
    lvar4 = vec3(lvar3, lvar2, lvar1);
    payload2 = lvar4;
}
