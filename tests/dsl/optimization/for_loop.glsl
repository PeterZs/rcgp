void sr(int arg0, int arg1, out float ret0)
{
    float lvar0;
    lvar0 = 0;
    int lvar1;
    lvar1 = 0;
    while (true) {
        if ((!(lvar1 < arg0))) {
            break;
        }
        lvar0 = (lvar0 + lvar1);
        lvar1 = (lvar1 + arg1);
    }
    ret0 = lvar0;
}
