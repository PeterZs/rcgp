#version 460

#extension GL_EXT_scalar_block_layout : require

layout (location = 0) smooth in float lin0;
layout (location = 1) noperspective in vec2 lin1;
layout (location = 2) flat in uvec4 lin2;

layout (location = 0) out vec3 lout0;

void main()
{
#ifdef __clang__
    float lvar0;
    lvar0 = lin0;
#elif defined(__GNUC__)
    uvec4 lvar0;
    lvar0 = uvec4(lin2);
#endif
    vec2 lvar1;
    lvar1 = vec2(lin1);
#ifdef __clang__
    uvec4 lvar2;
    lvar2 = uvec4(lin2);
#elif defined(__GNUC__)
    float lvar2;
    lvar2 = lin0;
#endif
    float lvar3;
    lvar3 = 1;
    vec3 lvar4;
    lvar4 = vec3(lvar3, lvar3, lvar3);
    lout0 = lvar4;
}
