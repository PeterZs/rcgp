#version 460

#extension GL_EXT_scalar_block_layout : require

void main()
{
    float lvar0;
    lvar0 = 1;
    vec4 lvar1;
    lvar1 = vec4(lvar0, lvar0, lvar0, lvar0);
    gl_Position = lvar1;
}
