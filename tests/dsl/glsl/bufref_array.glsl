#version 460

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout (buffer_reference, scalar) readonly buffer BufRef_scalar_vec2_array {
    vec2 value[];
};

layout (buffer_reference, scalar) readonly buffer BufRef_scalar_vec3_array {
    vec3 value[];
};

struct bufref_VertexData {
    BufRef_scalar_vec3_array positions;
    BufRef_scalar_vec3_array normals;
    BufRef_scalar_vec2_array uvs;
};

layout (location = 0) smooth in vec3 lin0;

layout (location = 0) out vec3 lout0;

layout (scalar, set = 0, binding = 0) readonly buffer Buffer0x0 {
    bufref_VertexData value[];
} r0b0;

void main()
{
    uint lvar0;
    lvar0 = 0;
    uint lvar1;
    lvar1 = 0;
    uint lvar2;
    lvar2 = 1;
    uint lvar3;
    lvar3 = 2;
    float lvar4;
    lvar4 = 0;
    vec3 lvar5;
    lvar5 = vec3(r0b0.value[lvar0].uvs.value[lvar3], lvar4);
    vec3 lvar6;
    lvar6 = (r0b0.value[lvar0].positions.value[lvar1] + r0b0.value[lvar0].normals.value[lvar2]);
    vec3 lvar7;
    lvar7 = (lvar6 + lvar5);
    lout0 = lvar7;
}
