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
    gl_Position = (pc.transform.value * vec4(0, 0, 0, 1));
}
