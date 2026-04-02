#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

hitAttributeEXT vec2 hit_attribute;

layout (set = 0, binding = 0) uniform accelerationStructureEXT r0b0;

layout (location = 0) rayPayloadInEXT vec3 payload0;

layout (location = 1) rayPayloadEXT float payload1;

layout (location = 2) rayPayloadEXT vec3 payload2;

void main()
{
    vec3 lvar0;
    lvar0 = gl_WorldRayDirectionEXT;
    float lvar1;
    lvar1 = gl_HitTEXT;
    vec3 lvar2;
    lvar2 = (lvar0 * lvar1);
    vec3 lvar3;
    lvar3 = gl_WorldRayOriginEXT;
    vec3 lvar4;
    lvar4 = (lvar3 + lvar2);
    float lvar5;
    lvar5 = 0;
    payload1 = lvar5;
    uint lvar6;
    lvar6 = 255;
    float lvar7;
    lvar7 = 100;
    float lvar8;
    lvar8 = 0.001;
    float lvar9;
    lvar9 = 0;
    float lvar10;
    lvar10 = 1;
    float lvar11;
    lvar11 = 0;
    vec3 lvar12;
    lvar12 = vec3(lvar11, lvar10, lvar9);
    int lvar13;
    lvar13 = 13;
    traceRayEXT(r0b0, lvar13, lvar6, 0, 1, 1, lvar4, lvar8, lvar12, lvar7, 1);
    vec3 lvar14;
    lvar14 = gl_WorldRayOriginEXT;
    vec3 lvar15;
    lvar15 = (lvar14 - lvar4);
    vec3 lvar16;
    lvar16 = normalize(lvar15);
    float lvar17;
    lvar17 = 0;
    float lvar18;
    lvar18 = 1;
    float lvar19;
    lvar19 = 0;
    vec3 lvar20;
    lvar20 = vec3(lvar19, lvar18, lvar17);
    float lvar21;
    lvar21 = dot(lvar16, lvar20);
    float lvar22;
    lvar22 = 2;
    vec3 lvar23;
    lvar23 = (lvar20 * lvar22);
    vec3 lvar24;
    lvar24 = (lvar23 * lvar21);
    vec3 lvar25;
    lvar25 = (lvar16 - lvar24);
    float lvar26;
    lvar26 = 0;
    vec3 lvar27;
    lvar27 = vec3(lvar26, lvar26, lvar26);
    payload2 = lvar27;
    uint lvar28;
    lvar28 = 255;
    float lvar29;
    lvar29 = 100;
    float lvar30;
    lvar30 = 0.001;
    int lvar31;
    lvar31 = 0;
    traceRayEXT(r0b0, lvar31, lvar28, 0, 1, 2, lvar4, lvar30, lvar25, lvar29, 2);
    float lvar32;
    lvar32 = 0.3;
    vec3 lvar33;
    lvar33 = (payload2 * lvar32);
    vec3 lvar34;
    lvar34 = vec3(payload1, payload1, payload1);
    vec3 lvar35;
    lvar35 = (lvar34 + lvar33);
    payload0 = lvar35;
}
