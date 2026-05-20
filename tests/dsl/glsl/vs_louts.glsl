#version 460

#extension GL_EXT_scalar_block_layout : require

layout (location = 0) smooth out vec3 lout0;
layout (location = 1) flat out uvec2 lout1;

void main()
{
    float lvar0;
    lvar0 = 1;
    vec3 lvar1;
    lvar1 = vec3(lvar0, lvar0, lvar0);
    vec3 lvar2;
    lvar2 = vec3(lvar1);
    lout0 = lvar2;
    uint lvar3;
#ifdef __clang__
    lvar3 = 1;
#elif defined(__GNUC__)
    lvar3 = 4;
#endif
    uint lvar4;
#ifdef __clang__
    lvar4 = 4;
#elif defined(__GNUC__)
    lvar4 = 1;
#endif
    uvec2 lvar5;
#ifdef __clang__
    lvar5 = uvec2(lvar3, lvar4);
#elif defined(__GNUC__)
    lvar5 = uvec2(lvar4, lvar3);
#endif
    uvec2 lvar6;
    lvar6 = uvec2(lvar5);
    lout1 = lvar6;
}
