#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

float powi(float base, int exp)
{
    if (exp == 0)
        return 0;
    else if (exp > 0)
    {
        while (exp - 1)
        {
            base*= base;
            --exp;
        }
        return base;
    }
    else
    {
        exp = -exp;
        while (exp - 1)
        {
            base/= base;
            --exp;
        }
        return base;
    }
}

#endif
