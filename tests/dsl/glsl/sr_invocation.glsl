#version 460

#extension GL_EXT_scalar_block_layout : require

#ifdef __clang__
struct rcgp_Ray {
#elif defined(__GNUC__)
struct Ray {
#endif
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
#ifdef __clang__
    float lvar3;
    lvar3 = arg0;
    uint lvar4;
    lvar4 = arg1;
#elif defined(__GNUC__)
    uint lvar3;
    lvar3 = arg1;
    float lvar4;
    lvar4 = arg0;
#endif
    float lvar5;
#ifdef __clang__
    lvar5 = lvar3;
#elif defined(__GNUC__)
    lvar5 = lvar4;
#endif
    vec3 lvar6;
    lvar6 = vec3(lvar5, lvar5, lvar5);
    uint lvar7;
    lvar7 = 13;
    uvec2 lvar8;
#ifdef __clang__
    lvar8 = uvec2(lvar4, lvar7);
#elif defined(__GNUC__)
    lvar8 = uvec2(lvar3, lvar7);
#endif
    uvec2 lvar9;
    lvar9 = uvec2(lvar8);
    vec3 lvar10;
    lvar10 = vec3(lvar6);
    ret0 = lvar10;
    ret1 = lvar9;
}

#ifdef __clang__
void sr3(float arg0, out rcgp_Ray ret0)
#elif defined(__GNUC__)
void sr3(float arg0, out Ray ret0)
#endif
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
#ifdef __clang__
    lvar16 = vec3(lvar14, lvar11, lvar15);
#elif defined(__GNUC__)
    lvar16 = vec3(lvar15, lvar11, lvar14);
#endif
    vec3 lvar17;
    lvar17 = normalize(lvar16);
    vec3 lvar18;
#ifdef __clang__
    lvar18 = vec3(lvar13);
#elif defined(__GNUC__)
    lvar18 = vec3(lvar17);
#endif
    vec3 lvar19;
    lvar19 = vec3(lvar18);
    vec3 lvar20;
#ifdef __clang__
    lvar20 = vec3(lvar17);
#elif defined(__GNUC__)
    lvar20 = vec3(lvar13);
#endif
    vec3 lvar21;
    lvar21 = vec3(lvar20);
    vec3 lvar22;
#ifdef __clang__
    lvar22 = vec3(lvar21);
#elif defined(__GNUC__)
    lvar22 = vec3(lvar19);
#endif
    vec3 lvar23;
#ifdef __clang__
    lvar23 = vec3(lvar19);
    ret0 = rcgp_Ray(lvar23, lvar22);
#elif defined(__GNUC__)
    lvar23 = vec3(lvar21);
    ret0 = Ray(lvar23, lvar22);
#endif
}

void main()
{
    float lvar24;
    lvar24 = 1;
    float lvar25;
    lvar25 = lvar24;
    vec3 lvar26;
    sr1(lvar25, lvar26);
#ifdef __clang__
    vec3 lvar27;
    lvar27 = vec3(lvar26);
#elif defined(__GNUC__)
    uint lvar27;
    lvar27 = 2;
#endif
    float lvar28;
    lvar28 = 1;
#ifdef __clang__
    uint lvar29;
    lvar29 = 2;
    float lvar30;
    lvar30 = lvar28;
    uint lvar31;
    lvar31 = lvar29;
    vec3 lvar32;
#elif defined(__GNUC__)
    float lvar29;
    lvar29 = lvar28;
    uint lvar30;
    lvar30 = lvar27;
    vec3 lvar31;
    uvec2 lvar32;
    sr2(lvar29, lvar30, lvar31, lvar32);
#endif
    uvec2 lvar33;
#ifdef __clang__
    sr2(lvar30, lvar31, lvar32, lvar33);
#elif defined(__GNUC__)
    lvar33 = uvec2(lvar31);
#endif
    vec3 lvar34;
    lvar34 = vec3(lvar32);
#ifdef __clang__
    uvec2 lvar35;
    lvar35 = uvec2(lvar33);
    uvec2 lvar36;
    lvar36 = uvec2(lvar35);
    vec3 lvar37;
    lvar37 = vec3(lvar34);
    float lvar38;
    lvar38 = 2;
    float lvar39;
    lvar39 = lvar38;
    rcgp_Ray lvar40;
    sr3(lvar39, lvar40);
#elif defined(__GNUC__)
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
#endif
    vec3 lvar41;
#ifdef __clang__
    lvar41 = vec3(lvar40.origin);
#elif defined(__GNUC__)
    lvar41 = vec3(lvar34);
#endif
    vec3 lvar42;
#ifdef __clang__
    lvar42 = vec3(lvar40.direction);
    vec3 lvar43;
    lvar43 = vec3(lvar42);
    vec3 lvar44;
    lvar44 = vec3(lvar41);
    uvec2 lvar45;
    lvar45 = uvec2(lvar36);
    vec3 lvar46;
    lvar46 = vec3(lvar37);
    vec3 lvar47;
    lvar47 = vec3(lvar27);
    lout0 = lvar47;
    lout1 = lvar46;
    lout2 = lvar45;
    lout3 = lvar44;
    lout4 = lvar43;
#elif defined(__GNUC__)
    lvar42 = vec3(lvar26);
    lout0 = lvar42;
    lout1 = lvar41;
    lout2 = lvar40;
    lout3 = lvar39;
    lout4 = lvar38;
#endif
}
