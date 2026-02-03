#version 460

#extension GL_EXT_scalar_block_layout : require

struct View {
    mat4 f0;
    mat4 f1;
    mat4 f2;
};

layout (location = 0) in vec3 lin0;

layout (location = 0) smooth out vec3 lout0;

layout (std430, push_constant) uniform PC {
    layout (offset = 0) View pc;
};

void main()
{
    vec3 lvar0;
    mat4 lvar1;
    mat4 lvar2;
    mat4 lvar3;
    mat4 lvar4;
    mat4 lvar5;
    mat4 lvar6;
    float lvar7;
    lvar7 = 1;
    vec4 lvar8;
    lvar8 = vec4(lin0, lvar7);
    gl_Position = ((pc.f2 * pc.f1) * (pc.f0 * lvar8));
    vec3 lvar9;
    lvar9 = vec3((pc.f0 * lvar8));
    lout0 = lvar9;
}
