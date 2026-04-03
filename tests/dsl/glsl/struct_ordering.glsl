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
    vec2 lvar0;
    lvar0 = vec2(lin0);
    vec3 lvar1;
    lvar1 = vec3(pc.payload.value);
    int lvar2;
    lvar2 = pc.payload.index;
    float lvar3;
    lvar3 = pc.weight;
    vec3 lvar4;
    lvar4 = vec3(lvar0, lvar3);
    vec3 lvar5;
    lvar5 = (lvar1 + lvar4);
    float lvar6;
    lvar6 = 1;
    vec4 lvar7;
    lvar7 = vec4(lvar5, lvar6);
    lout0 = lvar7;
}
