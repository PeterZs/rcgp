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
    vec3 lvar0;
    lvar0 = vec3(lin0);
    uint lvar1;
    lvar1 = 0;
    uint lvar2;
    lvar2 = lvar1;
    uint lvar3;
    lvar3 = 0;
    uint lvar4;
    lvar4 = lvar3;
    uint lvar5;
    lvar5 = 1;
    uint lvar6;
    lvar6 = lvar5;
    uint lvar7;
    lvar7 = 2;
    uint lvar8;
    lvar8 = lvar7;
#ifdef __clang__
    vec3 lvar9;
    lvar9 = (r0b0.value[lvar2].positions.value[lvar4] + r0b0.value[lvar2].normals.value[lvar6]);
    float lvar10;
    lvar10 = 0;
#elif defined(__GNUC__)
    float lvar9;
    lvar9 = 0;
    vec3 lvar10;
    lvar10 = vec3(r0b0.value[lvar2].uvs.value[lvar8], lvar9);
#endif
    vec3 lvar11;
#ifdef __clang__
    lvar11 = vec3(r0b0.value[lvar2].uvs.value[lvar8], lvar10);
#elif defined(__GNUC__)
    lvar11 = (r0b0.value[lvar2].positions.value[lvar4] + r0b0.value[lvar2].normals.value[lvar6]);
#endif
    vec3 lvar12;
#ifdef __clang__
    lvar12 = (lvar9 + lvar11);
#elif defined(__GNUC__)
    lvar12 = (lvar11 + lvar10);
#endif
    lout0 = lvar12;
}
