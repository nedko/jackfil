#include <math.h>

float exp2ap (float x)
{
    int i;

    i = (int)(floor (x));
    x -= i;
//    return ldexp (1 + x * (0.66 + 0.34 * x), i);
    return ldexp (1 + x * (0.6930 + x * (0.2416 + x * (0.0517 + x * 0.0137))), i);
}
