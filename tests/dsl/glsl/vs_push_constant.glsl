#version 460

#extension GL_EXT_scalar_block_layout : require

struct fwd_View {
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout (location = 0) in vec3 lin0;

layout (location = 0) smooth out vec3 lout0;

layout (std430, push_constant) uniform PC {
    layout (offset = 0) fwd_View pc;
};

void main()
{
    vec3 lvar0;
    lvar0 = vec3(lin0);
    float lvar1;
    lvar1 = 1;
    vec4 lvar2;
    lvar2 = vec4(lvar0, lvar1);
    vec4 lvar3;
    lvar3 = (pc.model * lvar2);
    mat4 lvar4;
    lvar4 = (pc.proj * pc.view);
    vec4 lvar5;
    lvar5 = (lvar4 * lvar3);
    gl_Position = lvar5;
    vec4 lvar6;
    lvar6 = vec4(lvar3);
    vec3 lvar7;
    lvar7 = vec3(lvar6);
    lout0 = lvar7;
}
