#version 460

#extension GL_EXT_scalar_block_layout : require

#ifdef __clang__
struct sanitization_value_or_index_rcgp_scalar_float {
#elif defined(__GNUC__)
struct sanitization_value_or_index_scalar_float {
#endif
    float value;
    int index;
};

#ifdef __clang__
struct sanitization_value_or_index_rcgp_vector_float_3 {
#elif defined(__GNUC__)
struct sanitization_value_or_index_vector_float_3 {
#endif
    vec3 value;
    int index;
};

struct sanitization_GenericMaterialInterface_Encoder {
#ifdef __clang__
    sanitization_value_or_index_rcgp_vector_float_3 albedo;
    sanitization_value_or_index_rcgp_vector_float_3 specular;
    sanitization_value_or_index_rcgp_scalar_float roughness;
#elif defined(__GNUC__)
    sanitization_value_or_index_vector_float_3 albedo;
    sanitization_value_or_index_vector_float_3 specular;
    sanitization_value_or_index_scalar_float roughness;
#endif
};

layout (location = 0) smooth in vec2 lin0;

layout (location = 0) out vec4 lout0;

layout (std430, push_constant) uniform PC {
    layout (offset = 0) sanitization_GenericMaterialInterface_Encoder pc;
};

void main()
{
#ifdef __clang__
    vec3 lvar0;
    lvar0 = vec3(pc.albedo.value);
    int lvar1;
    lvar1 = pc.albedo.index;
    vec3 lvar2;
    lvar2 = vec3(pc.specular.value);
    int lvar3;
    lvar3 = pc.specular.index;
    float lvar4;
    lvar4 = pc.roughness.value;
    int lvar5;
    lvar5 = pc.roughness.index;
    vec2 lvar6;
    lvar6 = vec2(lin0);
#elif defined(__GNUC__)
    vec2 lvar0;
    lvar0 = vec2(lin0);
    vec3 lvar1;
    lvar1 = vec3(pc.albedo.value);
    int lvar2;
    lvar2 = pc.albedo.index;
    vec3 lvar3;
    lvar3 = vec3(pc.specular.value);
    int lvar4;
    lvar4 = pc.specular.index;
    float lvar5;
    lvar5 = pc.roughness.value;
    int lvar6;
    lvar6 = pc.roughness.index;
#endif
    vec3 lvar7;
#ifdef __clang__
    lvar7 = vec3(lvar6, lvar4);
#elif defined(__GNUC__)
    lvar7 = vec3(lvar0, lvar5);
#endif
    vec3 lvar8;
#ifdef __clang__
    lvar8 = (lvar0 + lvar7);
#elif defined(__GNUC__)
    lvar8 = (lvar1 + lvar7);
#endif
    float lvar9;
    lvar9 = 1;
    vec4 lvar10;
    lvar10 = vec4(lvar8, lvar9);
    lout0 = lvar10;
}
