#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

hitAttributeEXT vec2 hit_attribute;

layout (location = 2) rayPayloadInEXT vec3 payload2;

void main()
{
    vec2 lvar0 = hit_attribute;
    payload2 = vec3(lvar0.x, lvar0.y, 1);
}
