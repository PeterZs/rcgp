#version 460

#extension GL_EXT_scalar_block_layout : require

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void b(float arg0, float arg1, float arg2, out float ret0)
{
    float lvar0;
#ifdef __clang__
    lvar0 = arg0;
#elif defined(__GNUC__)
    lvar0 = arg2;
#endif
    float lvar1;
    lvar1 = arg1;
    float lvar2;
#ifdef __clang__
    lvar2 = arg2;
#elif defined(__GNUC__)
    lvar2 = arg0;
#endif
    float lvar3;
#ifdef __clang__
    lvar3 = (lvar0 * lvar1);
#elif defined(__GNUC__)
    lvar3 = (lvar2 * lvar1);
#endif
    float lvar4;
#ifdef __clang__
    lvar4 = (lvar3 + lvar2);
#elif defined(__GNUC__)
    lvar4 = (lvar3 + lvar0);
#endif
    ret0 = lvar4;
}

void c(vec2 arg0, vec2 arg1, vec2 arg2, out vec2 ret0)
{
    vec2 lvar5;
#ifdef __clang__
    lvar5 = vec2(arg0);
#elif defined(__GNUC__)
    lvar5 = vec2(arg2);
#endif
    vec2 lvar6;
    lvar6 = vec2(arg1);
    vec2 lvar7;
#ifdef __clang__
    lvar7 = vec2(arg2);
#elif defined(__GNUC__)
    lvar7 = vec2(arg0);
#endif
    float lvar8;
#ifdef __clang__
    lvar8 = lvar5.x;
#elif defined(__GNUC__)
    lvar8 = lvar5.y;
#endif
    float lvar9;
#ifdef __clang__
    lvar9 = lvar6.x;
#elif defined(__GNUC__)
    lvar9 = lvar6.y;
#endif
    float lvar10;
    lvar10 = lvar7.y;
    float lvar11;
#ifdef __clang__
    lvar11 = lvar8;
#elif defined(__GNUC__)
    lvar11 = lvar10;
#endif
    float lvar12;
    lvar12 = lvar9;
    float lvar13;
#ifdef __clang__
    lvar13 = lvar10;
#elif defined(__GNUC__)
    lvar13 = lvar8;
#endif
    float lvar14;
    b(lvar11, lvar12, lvar13, lvar14);
    float lvar15;
#ifdef __clang__
    lvar15 = lvar14;
#elif defined(__GNUC__)
    lvar15 = lvar5.y;
#endif
    float lvar16;
#ifdef __clang__
    lvar16 = lvar5.y;
#elif defined(__GNUC__)
    lvar16 = lvar6.x;
#endif
    float lvar17;
#ifdef __clang__
    lvar17 = lvar6.y;
#elif defined(__GNUC__)
    lvar17 = lvar7.x;
#endif
    float lvar18;
#ifdef __clang__
    lvar18 = lvar7.y;
#elif defined(__GNUC__)
    lvar18 = lvar17;
#endif
    float lvar19;
    lvar19 = lvar16;
    float lvar20;
#ifdef __clang__
    lvar20 = lvar17;
#elif defined(__GNUC__)
    lvar20 = lvar15;
#endif
    float lvar21;
#ifdef __clang__
    lvar21 = lvar18;
    float lvar22;
    b(lvar19, lvar20, lvar21, lvar22);
    float lvar23;
    lvar23 = lvar22;
    vec2 lvar24;
    lvar24 = vec2(lvar15, lvar23);
    ret0 = lvar24;
#elif defined(__GNUC__)
    b(lvar18, lvar19, lvar20, lvar21);
    vec2 lvar22;
    lvar22 = vec2(lvar21, lvar14);
    ret0 = lvar22;
#endif
}

void a(vec4 arg0, vec4 arg1, vec4 arg2, out vec4 ret0)
{
#ifdef __GNUC__
    vec4 lvar23;
    lvar23 = vec4(arg2);
    vec4 lvar24;
    lvar24 = vec4(arg1);
#endif
    vec4 lvar25;
    lvar25 = vec4(arg0);
#ifdef __clang__
    vec4 lvar26;
    lvar26 = vec4(arg1);
    vec4 lvar27;
    lvar27 = vec4(arg2);
#elif defined(__GNUC__)
    vec2 lvar26;
    lvar26 = lvar23.zw;
    vec2 lvar27;
    lvar27 = lvar24.zw;
#endif
    vec2 lvar28;
#ifdef __clang__
    lvar28 = lvar25.xy;
#elif defined(__GNUC__)
    lvar28 = lvar25.zw;
#endif
    vec2 lvar29;
#ifdef __clang__
    lvar29 = lvar26.xy;
#elif defined(__GNUC__)
    lvar29 = vec2(lvar28);
#endif
    vec2 lvar30;
#ifdef __clang__
    lvar30 = lvar27.yy;
#elif defined(__GNUC__)
    lvar30 = vec2(lvar27);
#endif
    vec2 lvar31;
#ifdef __clang__
    lvar31 = vec2(lvar28);
#elif defined(__GNUC__)
    lvar31 = vec2(lvar26);
#endif
    vec2 lvar32;
#ifdef __clang__
    lvar32 = vec2(lvar29);
#elif defined(__GNUC__)
    c(lvar29, lvar30, lvar31, lvar32);
#endif
    vec2 lvar33;
#ifdef __clang__
    lvar33 = vec2(lvar30);
#elif defined(__GNUC__)
    lvar33 = lvar23.yy;
#endif
    vec2 lvar34;
#ifdef __clang__
    c(lvar31, lvar32, lvar33, lvar34);
#elif defined(__GNUC__)
    lvar34 = lvar24.xy;
#endif
    vec2 lvar35;
#ifdef __clang__
    lvar35 = vec2(lvar34);
#elif defined(__GNUC__)
    lvar35 = lvar25.xy;
#endif
    vec2 lvar36;
#ifdef __clang__
    lvar36 = lvar25.zw;
#elif defined(__GNUC__)
    lvar36 = vec2(lvar35);
#endif
    vec2 lvar37;
#ifdef __clang__
    lvar37 = lvar26.zw;
#elif defined(__GNUC__)
    lvar37 = vec2(lvar34);
#endif
    vec2 lvar38;
#ifdef __clang__
    lvar38 = lvar27.zw;
#elif defined(__GNUC__)
    lvar38 = vec2(lvar33);
#endif
    vec2 lvar39;
#ifdef __clang__
    lvar39 = vec2(lvar36);
    vec2 lvar40;
    lvar40 = vec2(lvar37);
    vec2 lvar41;
    lvar41 = vec2(lvar38);
    vec2 lvar42;
    c(lvar39, lvar40, lvar41, lvar42);
    vec2 lvar43;
    lvar43 = vec2(lvar42);
    vec4 lvar44;
    lvar44 = vec4(lvar35, lvar43);
    ret0 = lvar44;
#elif defined(__GNUC__)
    c(lvar36, lvar37, lvar38, lvar39);
    vec4 lvar40;
    lvar40 = vec4(lvar39, lvar32);
    ret0 = lvar40;
#endif
}

void main()
{
#ifdef __GNUC__
    float lvar41;
    lvar41 = 1;
    vec4 lvar42;
    lvar42 = vec4(lvar41, lvar41, lvar41, lvar41);
    float lvar43;
    lvar43 = 2;
    vec4 lvar44;
    lvar44 = vec4(lvar43, lvar43, lvar43, lvar43);
#endif
    float lvar45;
#ifdef __clang__
    lvar45 = 1;
#elif defined(__GNUC__)
    lvar45 = 3;
#endif
    vec4 lvar46;
    lvar46 = vec4(lvar45, lvar45, lvar45, lvar45);
#ifdef __clang__
    float lvar47;
    lvar47 = 2;
#elif defined(__GNUC__)
    vec4 lvar47;
    lvar47 = vec4(lvar46);
#endif
    vec4 lvar48;
#ifdef __clang__
    lvar48 = vec4(lvar47, lvar47, lvar47, lvar47);
    float lvar49;
    lvar49 = 3;
#elif defined(__GNUC__)
    lvar48 = vec4(lvar42);
    vec4 lvar49;
    lvar49 = vec4(lvar44);
#endif
    vec4 lvar50;
#ifdef __clang__
    lvar50 = vec4(lvar49, lvar49, lvar49, lvar49);
    vec4 lvar51;
    lvar51 = vec4(lvar50);
    vec4 lvar52;
    lvar52 = vec4(lvar46);
    vec4 lvar53;
    lvar53 = vec4(lvar48);
    vec4 lvar54;
    a(lvar51, lvar52, lvar53, lvar54);
    vec4 lvar55;
    lvar55 = vec4(lvar54);
#elif defined(__GNUC__)
    a(lvar47, lvar48, lvar49, lvar50);
#endif
}
