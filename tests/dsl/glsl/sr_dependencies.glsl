#version 460

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void b(float arg0, float arg1, float arg2, out float ret0)
{
    float lvar0;
    float lvar1;
    float lvar2;
    float lvar3;
    lvar3 = (arg0 * arg1);
    float lvar4;
    lvar4 = (lvar3 + arg2);
    ret0 = lvar4;
}

void c(vec2 arg0, vec2 arg1, vec2 arg2, out vec2 ret0)
{
    vec2 lvar5;
    vec2 lvar6;
    vec2 lvar7;
    float lvar8;
    lvar8 = arg2.y;
    float lvar9;
    lvar9 = arg1.y;
    float lvar10;
    lvar10 = arg0.y;
    float lvar11;
    b(lvar10, lvar9, lvar8, lvar11);
    float lvar12;
    float lvar13;
    lvar13 = arg2.y;
    float lvar14;
    lvar14 = arg1.x;
    float lvar15;
    lvar15 = arg0.x;
    float lvar16;
    b(lvar15, lvar14, lvar13, lvar16);
    float lvar17;
    vec2 lvar18;
    lvar18 = vec2(lvar16, lvar11);
    ret0 = lvar18;
}

void a(vec4 arg0, vec4 arg1, vec4 arg2, out vec4 ret0)
{
    vec4 lvar19;
    vec4 lvar20;
    vec4 lvar21;
    vec2 lvar22;
    lvar22 = arg2.zw;
    vec2 lvar23;
    lvar23 = arg1.zw;
    vec2 lvar24;
    lvar24 = arg0.zw;
    vec2 lvar25;
    c(lvar24, lvar23, lvar22, lvar25);
    vec2 lvar26;
    vec2 lvar27;
    lvar27 = arg2.yy;
    vec2 lvar28;
    lvar28 = arg1.xy;
    vec2 lvar29;
    lvar29 = arg0.xy;
    vec2 lvar30;
    c(lvar29, lvar28, lvar27, lvar30);
    vec2 lvar31;
    vec4 lvar32;
    lvar32 = vec4(lvar30, lvar25);
    ret0 = lvar32;
}

void main()
{
    float lvar33;
    lvar33 = 1;
    vec4 lvar34;
    lvar34 = vec4(lvar33, lvar33, lvar33, lvar33);
    float lvar35;
    lvar35 = 2;
    vec4 lvar36;
    lvar36 = vec4(lvar35, lvar35, lvar35, lvar35);
    float lvar37;
    lvar37 = 3;
    vec4 lvar38;
    lvar38 = vec4(lvar37, lvar37, lvar37, lvar37);
    vec4 lvar39;
    a(lvar38, lvar34, lvar36, lvar39);
    vec4 lvar40;
}
