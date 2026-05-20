#ifdef __clang__
void sr(float arg0, out rcgp_Ray ret0)
#elif defined(__GNUC__)
void sr(float arg0, out Ray ret0)
#endif
{
    float lvar0;
    lvar0 = arg0;
    float lvar1;
    lvar1 = 0;
    vec3 lvar2;
    lvar2 = vec3(lvar1, lvar1, lvar1);
    float lvar3;
    lvar3 = 1;
    float lvar4;
    lvar4 = 1;
    vec3 lvar5;
#ifdef __clang__
    lvar5 = vec3(lvar3, lvar0, lvar4);
#elif defined(__GNUC__)
    lvar5 = vec3(lvar4, lvar0, lvar3);
#endif
    vec3 lvar6;
    lvar6 = normalize(lvar5);
    vec3 lvar7;
#ifdef __clang__
    lvar7 = vec3(lvar2);
#elif defined(__GNUC__)
    lvar7 = vec3(lvar6);
#endif
    vec3 lvar8;
    lvar8 = vec3(lvar7);
    vec3 lvar9;
#ifdef __clang__
    lvar9 = vec3(lvar6);
#elif defined(__GNUC__)
    lvar9 = vec3(lvar2);
#endif
    vec3 lvar10;
    lvar10 = vec3(lvar9);
    vec3 lvar11;
#ifdef __clang__
    lvar11 = vec3(lvar10);
#elif defined(__GNUC__)
    lvar11 = vec3(lvar8);
#endif
    vec3 lvar12;
#ifdef __clang__
    lvar12 = vec3(lvar8);
    ret0 = rcgp_Ray(lvar12, lvar11);
#elif defined(__GNUC__)
    lvar12 = vec3(lvar10);
    ret0 = Ray(lvar12, lvar11);
#endif
}
