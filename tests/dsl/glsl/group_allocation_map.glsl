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
    vec2 lvar0;
    lvar0 = vec2(arg0);
    vec4 lvar1;
    lvar1 = texture(r1b0, lvar0);
    ret0 = lvar1;
}

void main()
{
    vec2 lvar2;
    lvar2 = vec2(lin0);
    vec2 lvar3;
    lvar3 = vec2(lvar2);
    vec4 lvar4;
    get_albedo(lvar3, lvar4);
#ifdef __clang__
    vec4 lvar5;
    lvar5 = vec4(lvar4);
    lout0 = lvar5;
#elif defined(__GNUC__)
    lout0 = lvar4;
#endif
}
