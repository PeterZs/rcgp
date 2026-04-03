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
    bufref_VertexData lvar0 = r0b0.value[0];
    lout0 = ((lvar0.positions.value[0] + lvar0.normals.value[1]) + vec3(lvar0.uvs.value[2], 0));
}
