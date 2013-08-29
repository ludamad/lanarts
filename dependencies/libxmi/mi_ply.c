#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"

/*
 * Written by Brian Kelleher; June 1986
 *
 * Draw a polygon (supplied as a polyline, i.e. an array of points), via
 * one of two scan conversion routines.
 */

void
#ifdef _HAVE_PROTOS
miFillPolygon_internal (miPaintedSet *paintedSet, const miGC *pGC, miPolygonShape shape, miCoordMode mode, int count, const miPoint *pPts)
#else
miFillPolygon_internal (paintedSet, pGC, shape, mode, count, pPts)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     miPolygonShape shape;
     miCoordMode mode;
     int count;
     const miPoint *pPts;
#endif
{
  miPoint *ppt = (miPoint *)NULL;
  const miPoint *q;
    
  /* ensure we have >=1 points */
  if (count <= 0)
    return;

  if (mode == MI_COORD_MODE_PREVIOUS)
    /* convert from relative to absolute coordinates */
    {
      int i;

      ppt = (miPoint *)mi_xmalloc (count * sizeof(miPoint));
      ppt[0] = pPts[0];
      for (i = 1; i < count; i++)
	{
	  ppt[i].x = ppt[i-1].x + pPts[i].x;
	  ppt[i].y = ppt[i-1].y + pPts[i].y;	  
	}
      q = ppt;
    }
  else
    q = pPts;

  switch ((int)shape)
    {
    case (int)MI_SHAPE_GENERAL:
    default:
      /* use general scan conversion routine */
      miFillGeneralPoly (paintedSet, pGC, count, q);
      break;
    case (int)MI_SHAPE_CONVEX:
      /* use special (faster) routine */
      miFillConvexPoly (paintedSet, pGC, count, q);
      break;
    }

  if (mode == MI_COORD_MODE_PREVIOUS)
    free (ppt);
}


