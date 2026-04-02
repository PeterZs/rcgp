#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

hitAttributeEXT vec2 hit_attribute;

layout (location = 0) rayPayloadInEXT vec3 payload0;

void main()
{
    vec2 lvar0 = hit_attribute;
    payload0 = vec3(lvar0.x, lvar0.y, 0);
}
