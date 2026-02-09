#version 460

struct fwdxPointLight {
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
    fwdxPointLight lights[];
} r0b0;

layout (set = 0, binding = 0) uniform sampler2D r0b0;

void main()
{
    vec2 lvar0;
    vec3 lvar1;
    vec3 lvar2;
    int lvar3;
    texture(r0b0, lin2);
    vec4 lvar4;
    lvar4 = texture(r0b0, lin2);
    vec3 lvar5;
    lvar5 = lvar4.xyz;
    float lvar6;
    lvar6 = 0;
    vec3 lvar7;
    lvar7 = vec3(lvar6, lvar6, lvar6);
    int lvar8;
    lvar8 = 0;
    while (true) {
        bool lvar9;
        lvar9 = (lvar8 < r0b0.count);
        bool lvar10;
        lvar10 = (!lvar9);
        if (lvar10) {
            break;
        }
        vec3 lvar11;
        vec3 lvar12;
        vec3 lvar13;
        float lvar14;
        vec3 lvar15;
        lvar15 = (r0b0.lights[lvar8].position - lin0);
        float lvar16;
        lvar16 = 0.0001;
        dot(lvar15, lvar15);
        float lvar17;
        lvar17 = dot(lvar15, lvar15);
        max(lvar17, lvar16);
        float lvar18;
        lvar18 = max(lvar17, lvar16);
        float lvar19;
        lvar19 = (r0b0.lights[lvar8].intensity / lvar18);
        normalize(lvar15);
        vec3 lvar20;
        lvar20 = normalize(lvar15);
        float lvar21;
        lvar21 = 0;
        dot(lin1, lvar20);
        float lvar22;
        lvar22 = dot(lin1, lvar20);
        max(lvar22, lvar21);
        float lvar23;
        lvar23 = max(lvar22, lvar21);
        vec3 lvar24;
        lvar24 = (lvar5 * lvar23);
        vec3 lvar25;
        lvar25 = (lvar24 * r0b0.lights[lvar8].color);
        vec3 lvar26;
        lvar26 = (lvar25 * lvar19);
        vec3 lvar27;
        lvar27 = (lvar7 + lvar26);
        lvar7 = lvar27;
        int lvar28;
        lvar28 = 1;
        int lvar29;
        lvar29 = (lvar8 + lvar28);
        lvar8 = lvar29;
    }
    lout0 = lvar7;
}
