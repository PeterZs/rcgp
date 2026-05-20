#version 460

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout (buffer_reference, scalar) readonly buffer BufRef_scalar_mat4 {
    mat4 value;
};

struct bufref_SceneData {
    BufRef_scalar_mat4 transform;
};

layout (std430, push_constant) uniform PC {
    layout (offset = 0) bufref_SceneData pc;
};

void main()
{
    float lvar0;
#ifdef __clang__
    lvar0 = 0;
#elif defined(__GNUC__)
    lvar0 = 1;
#endif
    float lvar1;
    lvar1 = 0;
    float lvar2;
    lvar2 = 0;
    float lvar3;
#ifdef __clang__
    lvar3 = 1;
#elif defined(__GNUC__)
    lvar3 = 0;
#endif
    vec4 lvar4;
#ifdef __clang__
    lvar4 = vec4(lvar0, lvar1, lvar2, lvar3);
#elif defined(__GNUC__)
    lvar4 = vec4(lvar3, lvar2, lvar1, lvar0);
#endif
    vec4 lvar5;
    lvar5 = (pc.transform.value * lvar4);
    gl_Position = lvar5;
}
