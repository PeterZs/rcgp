#version 460

#extension GL_EXT_scalar_block_layout : require

struct mapping_DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

layout (location = 0) smooth in vec2 lin0;

layout (location = 0) out vec4 lout0;

layout (set = 1, binding = 0) uniform sampler2D r1b0;

layout (std430, set = 0, binding = 0) buffer Buffer0x0 {
    mapping_DirectionalLight value[];
} r0b0;

void get_albedo(vec2 arg0, out vec4 ret0)
{
    vec4 lvar0;
    lvar0 = texture(r1b0, arg0);
    ret0 = lvar0;
}

void main()
{
    vec4 lvar1;
    get_albedo(lin0, lvar1);
    lout0 = lvar1;
}
