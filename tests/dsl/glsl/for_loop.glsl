void sr(int arg0, int arg1, out float ret0)
{
    int lvar0;
#ifdef __clang__
    lvar0 = arg0;
#elif defined(__GNUC__)
    lvar0 = arg1;
#endif
    int lvar1;
#ifdef __clang__
    lvar1 = arg1;
#elif defined(__GNUC__)
    lvar1 = arg0;
#endif
    float lvar2;
    lvar2 = 0;
    int lvar3;
    lvar3 = 0;
    while (true) {
        int lvar4;
#ifdef __clang__
        lvar4 = lvar3;
#elif defined(__GNUC__)
        lvar4 = lvar1;
#endif
        int lvar5;
#ifdef __clang__
        lvar5 = lvar0;
#elif defined(__GNUC__)
        lvar5 = lvar3;
#endif
        bool lvar6;
#ifdef __clang__
        lvar6 = (lvar4 < lvar5);
#elif defined(__GNUC__)
        lvar6 = (lvar5 < lvar4);
#endif
        bool lvar7;
        lvar7 = (!lvar6);
        bool lvar8;
        lvar8 = lvar7;
        bool lvar9;
        lvar9 = lvar8;
        if (lvar9) {
            break;
        }
        float lvar10;
        lvar10 = lvar3;
        float lvar11;
        lvar11 = (lvar2 + lvar10);
        lvar2 = lvar11;
        int lvar12;
#ifdef __clang__
        lvar12 = (lvar3 + lvar1);
#elif defined(__GNUC__)
        lvar12 = (lvar3 + lvar0);
#endif
        lvar3 = lvar12;
    }
    ret0 = lvar2;
}
