#version 460

#extension GL_EXT_scalar_block_layout : require

struct Ray {
    vec3 origin;
    vec3 direction;
};

layout (location = 0) smooth out vec3 lout0;
layout (location = 1) smooth out vec3 lout1;
layout (location = 2) smooth out uvec2 lout2;
layout (location = 3) smooth out vec3 lout3;
layout (location = 4) smooth out vec3 lout4;

void sr1(float arg0, out vec3 ret0)
{
    float lvar0;
    lvar0 = arg0;
    float lvar1;
    lvar1 = lvar0;
    vec3 lvar2;
    lvar2 = vec3(lvar1, lvar1, lvar1);
    ret0 = lvar2;
}

void sr2(float arg0, uint arg1, out vec3 ret0, out uvec2 ret1)
{
    uint lvar3;
    lvar3 = arg1;
    float lvar4;
    lvar4 = arg0;
    float lvar5;
    lvar5 = lvar4;
    vec3 lvar6;
    lvar6 = vec3(lvar5, lvar5, lvar5);
    uint lvar7;
    lvar7 = 13;
    uvec2 lvar8;
    lvar8 = uvec2(lvar3, lvar7);
    uvec2 lvar9;
    lvar9 = uvec2(lvar8);
    vec3 lvar10;
    lvar10 = vec3(lvar6);
    ret0 = lvar10;
    ret1 = lvar9;
}

void sr3(float arg0, out Ray ret0)
{
    float lvar11;
    lvar11 = arg0;
    float lvar12;
    lvar12 = 0;
    vec3 lvar13;
    lvar13 = vec3(lvar12, lvar12, lvar12);
    float lvar14;
    lvar14 = 1;
    float lvar15;
    lvar15 = 1;
    vec3 lvar16;
    lvar16 = vec3(lvar15, lvar11, lvar14);
    vec3 lvar17;
    lvar17 = normalize(lvar16);
    vec3 lvar18;
    lvar18 = vec3(lvar17);
    vec3 lvar19;
    lvar19 = vec3(lvar18);
    vec3 lvar20;
    lvar20 = vec3(lvar13);
    vec3 lvar21;
    lvar21 = vec3(lvar20);
    vec3 lvar22;
    lvar22 = vec3(lvar19);
    vec3 lvar23;
    lvar23 = vec3(lvar21);
    ret0 = Ray(lvar23, lvar22);
}

void main()
{
    float lvar24;
    lvar24 = 1;
    float lvar25;
    lvar25 = lvar24;
    vec3 lvar26;
    sr1(lvar25, lvar26);
    uint lvar27;
    lvar27 = 2;
    float lvar28;
    lvar28 = 1;
    float lvar29;
    lvar29 = lvar28;
    uint lvar30;
    lvar30 = lvar27;
    vec3 lvar31;
    uvec2 lvar32;
    sr2(lvar29, lvar30, lvar31, lvar32);
    uvec2 lvar33;
    lvar33 = uvec2(lvar31);
    vec3 lvar34;
    lvar34 = vec3(lvar32);
    float lvar35;
    lvar35 = 2;
    float lvar36;
    lvar36 = lvar35;
    Ray lvar37;
    sr3(lvar36, lvar37);
    vec3 lvar38;
    lvar38 = vec3(lvar37.direction);
    vec3 lvar39;
    lvar39 = vec3(lvar37.origin);
    uvec2 lvar40;
    lvar40 = uvec2(lvar33);
    vec3 lvar41;
    lvar41 = vec3(lvar34);
    vec3 lvar42;
    lvar42 = vec3(lvar26);
    lout0 = lvar42;
    lout1 = lvar41;
    lout2 = lvar40;
    lout3 = lvar39;
    lout4 = lvar38;
}
