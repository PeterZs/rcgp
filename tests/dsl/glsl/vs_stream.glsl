#version 460

#extension GL_EXT_scalar_block_layout : require

layout (location = 0) in vec3 lin0;

layout (location = 0) smooth out vec3 lout0;

void main()
{
    vec3 lvar0;
    lvar0 = vec3(lin0);
    float lvar1;
    lvar1 = 1;
    vec4 lvar2;
    lvar2 = vec4(lvar0, lvar1);
    gl_Position = lvar2;
    vec3 lvar3;
    lvar3 = vec3(lvar0);
    lout0 = lvar3;
}
