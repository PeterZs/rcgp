void sr()
{
    int lvar0;
    lvar0 = 12;
    int lvar1;
#ifdef __clang__
    lvar1 = lvar0;
#elif defined(__GNUC__)
    lvar1 = 11;
#endif
    int lvar2;
#ifdef __clang__
    lvar2 = 11;
#elif defined(__GNUC__)
    lvar2 = lvar0;
#endif
    bool lvar3;
#ifdef __clang__
    lvar3 = (lvar1 > lvar2);
    bool lvar4;
    lvar4 = lvar3;
    bool lvar5;
    lvar5 = lvar4;
    int lvar6;
    lvar6 = lvar0;
    int lvar7;
    lvar7 = 11;
#elif defined(__GNUC__)
    lvar3 = (lvar2 < lvar1);
    int lvar4;
    lvar4 = 5;
    int lvar5;
    lvar5 = lvar0;
    bool lvar6;
    lvar6 = (lvar5 > lvar4);
    bool lvar7;
    lvar7 = (lvar3 && lvar6);
#endif
    bool lvar8;
#ifdef __clang__
    lvar8 = (lvar6 < lvar7);
#elif defined(__GNUC__)
    lvar8 = lvar7;
#endif
    int lvar9;
#ifdef __clang__
    lvar9 = lvar0;
#elif defined(__GNUC__)
    lvar9 = 11;
#endif
    int lvar10;
#ifdef __clang__
    lvar10 = 5;
#elif defined(__GNUC__)
    lvar10 = lvar0;
#endif
    bool lvar11;
#ifdef __clang__
    lvar11 = (lvar9 > lvar10);
#elif defined(__GNUC__)
    lvar11 = (lvar10 > lvar9);
#endif
    bool lvar12;
#ifdef __clang__
    lvar12 = (lvar8 && lvar11);
#elif defined(__GNUC__)
    lvar12 = lvar11;
#endif
    bool lvar13;
    lvar13 = lvar12;
    bool lvar14;
#ifdef __clang__
    lvar14 = lvar5;
#elif defined(__GNUC__)
    lvar14 = lvar13;
#endif
    bool lvar15;
#ifdef __clang__
    lvar15 = lvar13;
#elif defined(__GNUC__)
    lvar15 = lvar8;
#endif
    bool lvar16;
    lvar16 = lvar14;
    if (lvar16) {
        int lvar17;
        lvar17 = 1;
        int lvar18;
        lvar18 = (lvar0 + lvar17);
        lvar0 = lvar18;
    }
    else if (lvar15) {
        int lvar19;
        lvar19 = 2;
        int lvar20;
        lvar20 = (lvar0 + lvar19);
        lvar0 = lvar20;
    }
    else {
        int lvar21;
        lvar21 = 3;
        int lvar22;
        lvar22 = (lvar0 + lvar21);
        lvar0 = lvar22;
    }
}
