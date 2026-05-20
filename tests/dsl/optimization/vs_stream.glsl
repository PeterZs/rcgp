#version 460

#extension GL_EXT_scalar_block_layout : require

layout (location = 0) in vec3 lin0;

layout (location = 0) smooth out vec3 lout0;

void main()
{
    vec3 lvar0 = vec3(lin0);
    gl_Position = vec4(lvar0, 1);
    lout0 = vec3(lvar0);
}
