#version 460

#extension GL_EXT_scalar_block_layout : require

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std430, set = 0, binding = 0) readonly buffer Buffer0x0 {
    vec4 value[];
} r0b0;

void main()
{
    int lvar0;
    lvar0 = r0b0.value.length();
}
