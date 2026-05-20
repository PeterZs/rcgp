#version 460

#extension GL_EXT_scalar_block_layout : require

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void b(float arg0, float arg1, float arg2, out float ret0)
{
    ret0 = ((arg0 * arg1) + arg2);
}

void c(vec2 arg0, vec2 arg1, vec2 arg2, out vec2 ret0)
{
#ifdef __clang__
    vec2 lvar0 = vec2(arg0);
#elif defined(__GNUC__)
    vec2 lvar0 = vec2(arg2);
#endif
    vec2 lvar1 = vec2(arg1);
#ifdef __clang__
    vec2 lvar2 = vec2(arg2);
#elif defined(__GNUC__)
    vec2 lvar2 = vec2(arg0);
#endif
    float lvar3;
#ifdef __clang__
    b(lvar0.x, lvar1.x, lvar2.y, lvar3);
#elif defined(__GNUC__)
    b(lvar2.y, lvar1.y, lvar0.y, lvar3);
#endif
    float lvar4;
#ifdef __clang__
    b(lvar0.y, lvar1.y, lvar2.y, lvar4);
    ret0 = vec2(lvar3, lvar4);
#elif defined(__GNUC__)
    b(lvar2.x, lvar1.x, lvar0.y, lvar4);
    ret0 = vec2(lvar4, lvar3);
#endif
}

void a(vec4 arg0, vec4 arg1, vec4 arg2, out vec4 ret0)
{
#ifdef __clang__
    vec4 lvar5 = vec4(arg0);
#elif defined(__GNUC__)
    vec4 lvar5 = vec4(arg2);
#endif
    vec4 lvar6 = vec4(arg1);
#ifdef __clang__
    vec4 lvar7 = vec4(arg2);
#elif defined(__GNUC__)
    vec4 lvar7 = vec4(arg0);
#endif
    vec2 lvar8;
#ifdef __clang__
    c(vec2(lvar5.xy), vec2(lvar6.xy), vec2(lvar7.yy), lvar8);
#elif defined(__GNUC__)
    c(vec2(lvar7.zw), vec2(lvar6.zw), vec2(lvar5.zw), lvar8);
#endif
    vec2 lvar9;
#ifdef __clang__
    c(vec2(lvar5.zw), vec2(lvar6.zw), vec2(lvar7.zw), lvar9);
    ret0 = vec4(vec2(lvar8), vec2(lvar9));
#elif defined(__GNUC__)
    c(vec2(lvar7.xy), vec2(lvar6.xy), vec2(lvar5.yy), lvar9);
    ret0 = vec4(lvar9, lvar8);
#endif
}

void main()
{
    vec4 lvar10;
    a(vec4(vec4(3, 3, 3, 3)), vec4(vec4(1, 1, 1, 1)), vec4(vec4(2, 2, 2, 2)), lvar10);
}
