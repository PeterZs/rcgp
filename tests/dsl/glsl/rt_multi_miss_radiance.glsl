#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

layout (location = 0) rayPayloadInEXT vec3 payload0;

void main()
{
    float lvar0;
#ifdef __clang__
    lvar0 = 0.5;
#elif defined(__GNUC__)
    lvar0 = 1;
#endif
    float lvar1;
    lvar1 = 0.7;
    float lvar2;
#ifdef __clang__
    lvar2 = 1;
#elif defined(__GNUC__)
    lvar2 = 0.5;
#endif
    vec3 lvar3;
#ifdef __clang__
    lvar3 = vec3(lvar0, lvar1, lvar2);
#elif defined(__GNUC__)
    lvar3 = vec3(lvar2, lvar1, lvar0);
#endif
    payload0 = lvar3;
}
