void sr(float arg0, uint arg1, out vec3 ret0, out uvec2 ret1)
{
#ifdef __clang__
    float lvar0;
    lvar0 = arg0;
    uint lvar1;
    lvar1 = arg1;
#elif defined(__GNUC__)
    uint lvar0;
    lvar0 = arg1;
    float lvar1;
    lvar1 = arg0;
#endif
    float lvar2;
#ifdef __clang__
    lvar2 = lvar0;
#elif defined(__GNUC__)
    lvar2 = lvar1;
#endif
    vec3 lvar3;
    lvar3 = vec3(lvar2, lvar2, lvar2);
    uint lvar4;
    lvar4 = 13;
    uvec2 lvar5;
#ifdef __clang__
    lvar5 = uvec2(lvar1, lvar4);
#elif defined(__GNUC__)
    lvar5 = uvec2(lvar0, lvar4);
#endif
    uvec2 lvar6;
    lvar6 = uvec2(lvar5);
    vec3 lvar7;
    lvar7 = vec3(lvar3);
    ret0 = lvar7;
    ret1 = lvar6;
}
