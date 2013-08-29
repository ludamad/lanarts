#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"

/* mi rectangles
   written by Todd Newman, with debts to all and sundry
   */

/* Very straightforward.  We let the low-level paint function invoked by
 * MI_PAINT_SPANS() worry about clipping to the destination.  The miGC
 * struct is unused.
 *
 * Note libxmi's convention: right edges and bottom edges of filled
 * polygons (including rectangles) are unpainted, so that adjacent polygons
 * will abut with no overlaps or gaps. */

void
#ifdef _HAVE_PROTOS
miFillRectangles_internal (miPaintedSet *paintedSet, const miGC *pGC, int nrects, const miRectangle *prectInit)
#else
miFillRectangles_internal (paintedSet, pGC, nrects, prectInit)
     miPaintedSet *paintedSet;
     const miGC *pGC;		/* unused */
     int nrects; 
     const miRectangle *prectInit;
#endif
{
  miPoint *ppt;
  miPoint *pptFirst;
  int xorg, yorg;
  unsigned int *pw, *pwFirst;
  unsigned int height, width;
  const miRectangle *prect; 

  /* ensure we have >=1 rects to fill */
  if (nrects <= 0)
    return;

  prect = prectInit;
  while (nrects--)
    {
      height = prect->height;
      width = prect->width;
      pptFirst = (miPoint *)mi_xmalloc (height * sizeof(miPoint));
      pwFirst = (unsigned int *)mi_xmalloc (height * sizeof(unsigned int));
      ppt = pptFirst;
      pw = pwFirst;

      xorg = prect->x;
      yorg = prect->y;
      while (height--)
	{
	  *pw++ = width;
	  ppt->x = xorg;
	  ppt->y = yorg;
	  ppt++;
	  yorg++;
	}

      /* paint to paintedSet, or if that's NULL, to canvas */
      MI_PAINT_SPANS(paintedSet, pGC->pixels[1], (int)height, pptFirst, pwFirst)

      prect++;
    }
}
