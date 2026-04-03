#version 460

#extension GL_EXT_scalar_block_layout : require

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void b(float arg0, float arg1, float arg2, out float ret0)
{
    float lvar0;
    lvar0 = arg2;
    float lvar1;
    lvar1 = arg1;
    float lvar2;
    lvar2 = arg0;
    float lvar3;
    lvar3 = (lvar2 * lvar1);
    float lvar4;
    lvar4 = (lvar3 + lvar0);
    ret0 = lvar4;
}

void c(vec2 arg0, vec2 arg1, vec2 arg2, out vec2 ret0)
{
    vec2 lvar5;
    lvar5 = vec2(arg2);
    vec2 lvar6;
    lvar6 = vec2(arg1);
    vec2 lvar7;
    lvar7 = vec2(arg0);
    float lvar8;
    lvar8 = lvar5.y;
    float lvar9;
    lvar9 = lvar6.y;
    float lvar10;
    lvar10 = lvar7.y;
    float lvar11;
    lvar11 = lvar10;
    float lvar12;
    lvar12 = lvar9;
    float lvar13;
    lvar13 = lvar8;
    float lvar14;
    b(lvar11, lvar12, lvar13, lvar14);
    float lvar15;
    lvar15 = lvar5.y;
    float lvar16;
    lvar16 = lvar6.x;
    float lvar17;
    lvar17 = lvar7.x;
    float lvar18;
    lvar18 = lvar17;
    float lvar19;
    lvar19 = lvar16;
    float lvar20;
    lvar20 = lvar15;
    float lvar21;
    b(lvar18, lvar19, lvar20, lvar21);
    vec2 lvar22;
    lvar22 = vec2(lvar21, lvar14);
    ret0 = lvar22;
}

void a(vec4 arg0, vec4 arg1, vec4 arg2, out vec4 ret0)
{
    vec4 lvar23;
    lvar23 = vec4(arg2);
    vec4 lvar24;
    lvar24 = vec4(arg1);
    vec4 lvar25;
    lvar25 = vec4(arg0);
    vec2 lvar26;
    lvar26 = lvar23.zw;
    vec2 lvar27;
    lvar27 = lvar24.zw;
    vec2 lvar28;
    lvar28 = lvar25.zw;
    vec2 lvar29;
    lvar29 = vec2(lvar28);
    vec2 lvar30;
    lvar30 = vec2(lvar27);
    vec2 lvar31;
    lvar31 = vec2(lvar26);
    vec2 lvar32;
    c(lvar29, lvar30, lvar31, lvar32);
    vec2 lvar33;
    lvar33 = lvar23.yy;
    vec2 lvar34;
    lvar34 = lvar24.xy;
    vec2 lvar35;
    lvar35 = lvar25.xy;
    vec2 lvar36;
    lvar36 = vec2(lvar35);
    vec2 lvar37;
    lvar37 = vec2(lvar34);
    vec2 lvar38;
    lvar38 = vec2(lvar33);
    vec2 lvar39;
    c(lvar36, lvar37, lvar38, lvar39);
    vec4 lvar40;
    lvar40 = vec4(lvar39, lvar32);
    ret0 = lvar40;
}

void main()
{
    float lvar41;
    lvar41 = 1;
    vec4 lvar42;
    lvar42 = vec4(lvar41, lvar41, lvar41, lvar41);
    float lvar43;
    lvar43 = 2;
    vec4 lvar44;
    lvar44 = vec4(lvar43, lvar43, lvar43, lvar43);
    float lvar45;
    lvar45 = 3;
    vec4 lvar46;
    lvar46 = vec4(lvar45, lvar45, lvar45, lvar45);
    vec4 lvar47;
    lvar47 = vec4(lvar46);
    vec4 lvar48;
    lvar48 = vec4(lvar42);
    vec4 lvar49;
    lvar49 = vec4(lvar44);
    vec4 lvar50;
    a(lvar47, lvar48, lvar49, lvar50);
}
