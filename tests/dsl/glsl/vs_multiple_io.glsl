#version 460

#extension GL_EXT_scalar_block_layout : require

layout (location = 0) in vec3 lin0;
layout (location = 1) in vec3 lin1;
layout (location = 2) in vec2 lin2;

layout (location = 0) smooth out vec3 lout0;
layout (location = 1) smooth out vec3 lout1;
layout (location = 2) smooth out vec2 lout2;

void main()
{
#ifdef __clang__
    vec3 lvar0;
    lvar0 = vec3(lin0);
#elif defined(__GNUC__)
    vec2 lvar0;
    lvar0 = vec2(lin2);
#endif
    vec3 lvar1;
    lvar1 = vec3(lin1);
#ifdef __clang__
    vec2 lvar2;
    lvar2 = vec2(lin2);
#elif defined(__GNUC__)
    vec3 lvar2;
    lvar2 = vec3(lin0);
#endif
    vec3 lvar3;
#ifdef __clang__
    lvar3 = vec3(lvar0);
#elif defined(__GNUC__)
    lvar3 = vec3(lvar2);
#endif
    lout0 = lvar3;
    vec3 lvar4;
    lvar4 = vec3(lvar1);
    lout1 = lvar4;
    vec2 lvar5;
#ifdef __clang__
    lvar5 = vec2(lvar2);
#elif defined(__GNUC__)
    lvar5 = vec2(lvar0);
#endif
    lout2 = lvar5;
}
