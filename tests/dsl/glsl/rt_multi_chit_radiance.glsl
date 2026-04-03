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
    vec2 lvar0;
    lvar0 = vec2(hit_attribute);
    vec3 lvar1;
    lvar1 = gl_WorldRayDirectionEXT;
    float lvar2;
    lvar2 = gl_HitTEXT;
    vec3 lvar3;
    lvar3 = (lvar1 * lvar2);
    vec3 lvar4;
    lvar4 = gl_WorldRayOriginEXT;
    vec3 lvar5;
    lvar5 = (lvar4 + lvar3);
    float lvar6;
    lvar6 = 0;
    payload1 = lvar6;
    uint lvar7;
    lvar7 = 255;
    float lvar8;
    lvar8 = 100;
    float lvar9;
    lvar9 = 0.001;
    vec3 lvar10;
    lvar10 = vec3(lvar5);
    float lvar11;
    lvar11 = 0;
    float lvar12;
    lvar12 = 1;
    float lvar13;
    lvar13 = 0;
    vec3 lvar14;
    lvar14 = vec3(lvar13, lvar12, lvar11);
    int lvar15;
    lvar15 = 13;
    traceRayEXT(r0b0, lvar15, lvar7, 0, 1, 1, lvar10, lvar9, lvar14, lvar8, 1);
    vec3 lvar16;
    lvar16 = gl_WorldRayOriginEXT;
    vec3 lvar17;
    lvar17 = (lvar16 - lvar5);
    vec3 lvar18;
    lvar18 = normalize(lvar17);
    float lvar19;
    lvar19 = 0;
    float lvar20;
    lvar20 = 1;
    float lvar21;
    lvar21 = 0;
    vec3 lvar22;
    lvar22 = vec3(lvar21, lvar20, lvar19);
    float lvar23;
    lvar23 = dot(lvar18, lvar22);
    float lvar24;
    lvar24 = 2;
    float lvar25;
    lvar25 = lvar24;
    vec3 lvar26;
    lvar26 = (lvar22 * lvar25);
    float lvar27;
    lvar27 = lvar23;
    vec3 lvar28;
    lvar28 = (lvar26 * lvar27);
    vec3 lvar29;
    lvar29 = (lvar18 - lvar28);
    float lvar30;
    lvar30 = 0;
    vec3 lvar31;
    lvar31 = vec3(lvar30, lvar30, lvar30);
    payload2 = lvar31;
    uint lvar32;
    lvar32 = 255;
    float lvar33;
    lvar33 = 100;
    float lvar34;
    lvar34 = 0.001;
    vec3 lvar35;
    lvar35 = vec3(lvar5);
    vec3 lvar36;
    lvar36 = vec3(lvar29);
    int lvar37;
    lvar37 = 0;
    traceRayEXT(r0b0, lvar37, lvar32, 0, 1, 2, lvar35, lvar34, lvar36, lvar33, 2);
    float lvar38;
    lvar38 = 0.3;
    float lvar39;
    lvar39 = lvar38;
    vec3 lvar40;
    lvar40 = (payload2 * lvar39);
    float lvar41;
    lvar41 = payload1;
    vec3 lvar42;
    lvar42 = vec3(lvar41, lvar41, lvar41);
    vec3 lvar43;
    lvar43 = (lvar42 + lvar40);
    payload0 = lvar43;
}
