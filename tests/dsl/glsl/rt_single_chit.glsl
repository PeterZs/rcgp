#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

hitAttributeEXT vec2 hit_attribute;

layout (location = 0) rayPayloadInEXT vec3 payload0;

void main()
{
    float lvar0;
    lvar0 = 0;
    float lvar1;
    lvar1 = hit_attribute.y;
    float lvar2;
    lvar2 = hit_attribute.x;
    vec3 lvar3;
    lvar3 = vec3(lvar2, lvar1, lvar0);
    payload0 = lvar3;
}
