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
#ifdef __clang__
    lvar1 = lvar0.x;
#elif defined(__GNUC__)
    lvar1 = 1;
#endif
    float lvar2;
    lvar2 = lvar0.y;
    float lvar3;
#ifdef __clang__
    lvar3 = 1;
#elif defined(__GNUC__)
    lvar3 = lvar0.x;
#endif
    vec3 lvar4;
#ifdef __clang__
    lvar4 = vec3(lvar1, lvar2, lvar3);
#elif defined(__GNUC__)
    lvar4 = vec3(lvar3, lvar2, lvar1);
#endif
    payload2 = lvar4;
}
