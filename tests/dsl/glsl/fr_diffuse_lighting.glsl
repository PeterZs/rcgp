#version 460

#extension GL_EXT_scalar_block_layout : require

struct fwd_PointLight {
    vec3 position;
    vec3 color;
    vec3 velocity;
    float intensity;
};

layout (location = 0) smooth in vec3 lin0;
layout (location = 1) smooth in vec3 lin1;
layout (location = 2) smooth in vec2 lin2;

layout (location = 0) out vec3 lout0;

layout (std430, set = 0, binding = 0) readonly buffer Buffer0x0 {
    int count;
    fwd_PointLight lights[];
} r0b0;

layout (set = 0, binding = 0) uniform sampler2D r0b0;

layout (set = 0, binding = 1) uniform sampler2D r0b1;

layout (set = 0, binding = 2) uniform sampler2D r0b2;

void main()
{
    vec2 lvar0;
    lvar0 = vec2(lin2);
    vec3 lvar1;
    lvar1 = vec3(lin1);
    vec3 lvar2;
    lvar2 = vec3(lin0);
    int lvar3;
    lvar3 = r0b0.count;
    vec4 lvar4;
    lvar4 = texture(r0b0, lvar0);
    vec3 lvar5;
    lvar5 = lvar4.xyz;
    float lvar6;
    lvar6 = 0;
    vec3 lvar7;
    lvar7 = vec3(lvar6, lvar6, lvar6);
    int lvar8;
    lvar8 = 0;
    while (true) {
        int lvar9;
        lvar9 = lvar3;
        int lvar10;
        lvar10 = lvar8;
        bool lvar11;
        lvar11 = (lvar10 < lvar9);
        bool lvar12;
        lvar12 = (!lvar11);
        bool lvar13;
        lvar13 = lvar12;
        bool lvar14;
        lvar14 = lvar13;
        if (lvar14) {
            break;
        }
        int lvar15;
        lvar15 = lvar8;
        vec3 lvar16;
        lvar16 = (r0b0.lights[lvar15].position - lvar2);
        float lvar17;
        lvar17 = 0.0001;
        float lvar18;
        lvar18 = dot(lvar16, lvar16);
        float lvar19;
        lvar19 = lvar18;
        float lvar20;
        lvar20 = lvar17;
        float lvar21;
        lvar21 = max(lvar19, lvar20);
        float lvar22;
        lvar22 = (r0b0.lights[lvar15].intensity / lvar21);
        vec3 lvar23;
        lvar23 = normalize(lvar16);
        float lvar24;
        lvar24 = 0;
        float lvar25;
        lvar25 = dot(lvar1, lvar23);
        float lvar26;
        lvar26 = lvar25;
        float lvar27;
        lvar27 = lvar24;
        float lvar28;
        lvar28 = max(lvar26, lvar27);
        float lvar29;
        lvar29 = lvar28;
        vec3 lvar30;
        lvar30 = (lvar5 * lvar29);
        vec3 lvar31;
        lvar31 = (lvar30 * r0b0.lights[lvar15].color);
        float lvar32;
        lvar32 = lvar22;
        vec3 lvar33;
        lvar33 = (lvar31 * lvar32);
        vec3 lvar34;
        lvar34 = (lvar7 + lvar33);
        lvar7 = lvar34;
        int lvar35;
        lvar35 = lvar8;
        int lvar36;
        lvar36 = 1;
        int lvar37;
        lvar37 = (lvar8 + lvar36);
        lvar8 = lvar37;
    }
    lout0 = lvar7;
}
