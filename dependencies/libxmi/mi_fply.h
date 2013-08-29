#define EPSILON	0.000001
#define ISEQUAL(a,b) (FABS((a) - (b)) <= EPSILON)
#define UNEQUAL(a,b) (FABS((a) - (b)) > EPSILON)
#define PTISEQUAL(a,b) (ISEQUAL(a.x,b.x) && ISEQUAL(a.y,b.y))

/* Point with sub-pixel positioning.  In this case we use doubles, but
 * see mi_fplycon.c for other possibilities.
 */
typedef struct
{
  double	x, y;
} SppPoint;

/* Arc with sub-pixel positioning. */
typedef struct 
{
  double	x, y, width, height;
  double	angle1, angle2;
} SppArc;

extern void miFillSppPoly ____P((miPaintedSet *paintedSet, miPixel pixel, int count, const SppPoint *ptsIn, int xTrans, int yTrans, double xFtrans, double yFtrans));
