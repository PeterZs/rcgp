#ifdef __clang__
void sr(float arg0, out rcgp_Ray ret0)
#elif defined(__GNUC__)
void sr(float arg0, out Ray ret0)
#endif
{
#ifdef __clang__
    ret0 = rcgp_Ray(vec3(vec3(vec3(vec3(0, 0, 0)))), vec3(vec3(vec3(normalize(vec3(1, arg0, 1))))));
#elif defined(__GNUC__)
    ret0 = Ray(vec3(vec3(vec3(vec3(0, 0, 0)))), vec3(vec3(vec3(normalize(vec3(1, arg0, 1))))));
#endif
}
