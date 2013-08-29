/* Stuff needed for drawing thin (zero width) lines */

#define X_AXIS	0
#define Y_AXIS	1

#define OUT_LEFT  0x08
#define OUT_RIGHT 0x04
#define OUT_ABOVE 0x02
#define OUT_BELOW 0x01

#define MI_OUTCODES(outcode, x, y, xmin, ymin, xmax, ymax) \
{\
     if (x < xmin) outcode |= OUT_LEFT;\
     if (x > xmax) outcode |= OUT_RIGHT;\
     if (y < ymin) outcode |= OUT_ABOVE;\
     if (y > ymax) outcode |= OUT_BELOW;\
}

#define round(dividend, divisor) \
( (((dividend)<<1) + (divisor)) / ((divisor)<<1) )

#define ceiling(m,n)  (((m)-1)/(n) + 1)

#define SWAPINT(i, j) \
{  int _t = i;  i = j;  j = _t; }

#define SWAPINT_PAIR(x1, y1, x2, y2)\
{   int t = x1;  x1 = x2;  x2 = t;\
        t = y1;  y1 = y2;  y2 = t;\
}

#define AbsDeltaAndSign(_p2, _p1, _absdelta, _sign) \
    (_sign) = 1; \
    (_absdelta) = (_p2) - (_p1); \
    if ( (_absdelta) < 0) { (_absdelta) = -(_absdelta); (_sign) = -1; }

#ifndef FIXUP_X_MAJOR_ERROR
#define FIXUP_X_MAJOR_ERROR(_e, _signdx, _signdy) \
    (_e) -= ( (_signdx) < 0)
#endif

#ifndef FIXUP_Y_MAJOR_ERROR
#define FIXUP_Y_MAJOR_ERROR(_e, _signdx, _signdy) \
    (_e) -= ( (_signdy) < 0)
#endif

