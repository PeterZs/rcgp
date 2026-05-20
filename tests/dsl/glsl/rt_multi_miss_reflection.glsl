#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

layout (location = 2) rayPayloadInEXT vec3 payload2;

void main()
{
    float lvar0;
    lvar0 = 0.1;
    float lvar1;
    lvar1 = 0.1;
    float lvar2;
    lvar2 = 0.1;
    vec3 lvar3;
#ifdef __clang__
    lvar3 = vec3(lvar0, lvar1, lvar2);
#elif defined(__GNUC__)
    lvar3 = vec3(lvar2, lvar1, lvar0);
#endif
    payload2 = lvar3;
}
