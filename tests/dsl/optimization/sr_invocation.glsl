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
    ret0 = vec3(arg0, arg0, arg0);
}

void sr2(float arg0, uint arg1, out vec3 ret0, out uvec2 ret1)
{
    ret0 = vec3(vec3(arg0, arg0, arg0));
    ret1 = uvec2(uvec2(arg1, 13));
}

#ifdef __clang__
void sr3(float arg0, out rcgp_Ray ret0)
#elif defined(__GNUC__)
void sr3(float arg0, out Ray ret0)
#endif
{
#ifdef __clang__
    ret0 = rcgp_Ray(vec3(vec3(vec3(vec3(0, 0, 0)))), vec3(vec3(vec3(normalize(vec3(1, arg0, 1))))));
#elif defined(__GNUC__)
    ret0 = Ray(vec3(vec3(vec3(vec3(0, 0, 0)))), vec3(vec3(vec3(normalize(vec3(1, arg0, 1))))));
#endif
}

void main()
{
    vec3 lvar0;
    sr1(1, lvar0);
    vec3 lvar1;
    uvec2 lvar2;
    sr2(1, 2, lvar1, lvar2);
#ifdef __clang__
    rcgp_Ray lvar3;
#elif defined(__GNUC__)
    Ray lvar3;
#endif
    sr3(2, lvar3);
#ifdef __clang__
    lout0 = vec3(vec3(lvar0));
    lout1 = vec3(vec3(vec3(lvar1)));
    lout2 = uvec2(uvec2(uvec2(lvar2)));
    lout3 = vec3(vec3(lvar3.origin));
    lout4 = vec3(vec3(lvar3.direction));
#elif defined(__GNUC__)
    lout0 = vec3(lvar0);
    lout1 = vec3(vec3(lvar2));
    lout2 = uvec2(uvec2(lvar1));
    lout3 = vec3(lvar3.origin);
    lout4 = vec3(lvar3.direction);
#endif
}
