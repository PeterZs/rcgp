#version 460

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void b(float arg0, float arg1, float arg2, out float ret0)
{
    float lvar0;
    float lvar1;
    float lvar2;
    ret0 = ((arg0 * arg1) + arg2);
}

void c(vec2 arg0, vec2 arg1, vec2 arg2, out vec2 ret0)
{
    vec2 lvar3;
    vec2 lvar4;
    vec2 lvar5;
    float lvar6;
    lvar6 = arg2.y;
    float lvar7;
    lvar7 = arg1.y;
    float lvar8;
    lvar8 = arg0.y;
    float lvar9;
    b(lvar8, lvar7, lvar6, lvar9);
    float lvar10;
    float lvar11;
    lvar11 = arg2.y;
    float lvar12;
    lvar12 = arg1.x;
    float lvar13;
    lvar13 = arg0.x;
    float lvar14;
    b(lvar13, lvar12, lvar11, lvar14);
    float lvar15;
    vec2 lvar16;
    lvar16 = vec2(lvar14, lvar9);
    ret0 = lvar16;
}

void a(vec4 arg0, vec4 arg1, vec4 arg2, out vec4 ret0)
{
    vec4 lvar17;
    vec4 lvar18;
    vec4 lvar19;
    vec2 lvar20;
    lvar20 = arg2.zw;
    vec2 lvar21;
    lvar21 = arg1.zw;
    vec2 lvar22;
    lvar22 = arg0.zw;
    vec2 lvar23;
    c(lvar22, lvar21, lvar20, lvar23);
    vec2 lvar24;
    vec2 lvar25;
    lvar25 = arg2.yy;
    vec2 lvar26;
    lvar26 = arg1.xy;
    vec2 lvar27;
    lvar27 = arg0.xy;
    vec2 lvar28;
    c(lvar27, lvar26, lvar25, lvar28);
    vec2 lvar29;
    vec4 lvar30;
    lvar30 = vec4(lvar28, lvar23);
    ret0 = lvar30;
}

void main()
{
    float lvar31;
    lvar31 = 1;
    vec4 lvar32;
    lvar32 = vec4(lvar31, lvar31, lvar31, lvar31);
    float lvar33;
    lvar33 = 2;
    vec4 lvar34;
    lvar34 = vec4(lvar33, lvar33, lvar33, lvar33);
    float lvar35;
    lvar35 = 3;
    vec4 lvar36;
    lvar36 = vec4(lvar35, lvar35, lvar35, lvar35);
    vec4 lvar37;
    a(lvar36, lvar32, lvar34, lvar37);
    vec4 lvar38;
}
