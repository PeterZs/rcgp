#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require

layout (set = 0, binding = 0) uniform accelerationStructureEXT r0b0;

layout (set = 0, binding = 0) uniform writeonly image2D r0b0;

layout (location = 0) rayPayloadEXT vec3 payload0;

void main()
{
    uvec3 lvar0;
    lvar0 = gl_LaunchIDEXT;
    float lvar1;
    lvar1 = 0;
    vec3 lvar2;
    lvar2 = vec3(lvar1, lvar1, lvar1);
    payload0 = lvar2;
    uint lvar3;
    lvar3 = 255;
    float lvar4;
    lvar4 = 100;
    float lvar5;
    lvar5 = 0.001;
    float lvar6;
    lvar6 = 0;
    vec3 lvar7;
    lvar7 = vec3(lvar6, lvar6, lvar6);
    float lvar8;
    lvar8 = 1;
    float lvar9;
    lvar9 = 0;
    float lvar10;
    lvar10 = 0;
    vec3 lvar11;
    lvar11 = vec3(lvar10, lvar9, lvar8);
    int lvar12;
    lvar12 = 1;
    traceRayEXT(r0b0, lvar12, lvar3, 0, 1, 0, lvar7, lvar5, lvar11, lvar4, 0);
    float lvar13;
    lvar13 = 0;
    vec4 lvar14;
    lvar14 = vec4(payload0, lvar13);
    uvec2 lvar15;
    lvar15 = lvar0.xy;
    ivec2 lvar16;
    lvar16 = ivec2(lvar15);
    imageStore(r0b0, lvar16, lvar14);
}
