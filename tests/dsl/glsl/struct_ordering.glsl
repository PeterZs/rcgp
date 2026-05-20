#version 460

#extension GL_EXT_scalar_block_layout : require

struct struct_sorting_ZLeaf {
    vec3 value;
    int index;
};

struct struct_sorting_AParent {
    struct_sorting_ZLeaf payload;
    float weight;
};

layout (location = 0) smooth in vec2 lin0;

layout (location = 0) out vec4 lout0;

layout (std430, push_constant) uniform PC {
    layout (offset = 0) struct_sorting_AParent pc;
};

void main()
{
#ifdef __clang__
    vec3 lvar0;
    lvar0 = vec3(pc.payload.value);
    int lvar1;
    lvar1 = pc.payload.index;
    float lvar2;
    lvar2 = pc.weight;
    vec2 lvar3;
    lvar3 = vec2(lin0);
#elif defined(__GNUC__)
    vec2 lvar0;
    lvar0 = vec2(lin0);
    vec3 lvar1;
    lvar1 = vec3(pc.payload.value);
    int lvar2;
    lvar2 = pc.payload.index;
    float lvar3;
    lvar3 = pc.weight;
#endif
    vec3 lvar4;
#ifdef __clang__
    lvar4 = vec3(lvar3, lvar2);
#elif defined(__GNUC__)
    lvar4 = vec3(lvar0, lvar3);
#endif
    vec3 lvar5;
#ifdef __clang__
    lvar5 = (lvar0 + lvar4);
#elif defined(__GNUC__)
    lvar5 = (lvar1 + lvar4);
#endif
    float lvar6;
    lvar6 = 1;
    vec4 lvar7;
    lvar7 = vec4(lvar5, lvar6);
    lout0 = lvar7;
}
