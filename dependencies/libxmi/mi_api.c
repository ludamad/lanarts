/* This file defines the core libxmi API, consisting of:

   1. miDrawPoints, miDrawLines, miFillPolygon.
   2. miDrawRectangles, miFillRectangles.
   3. miDrawArcs, miFillArcs.  Also the reentrant miDrawArcs_r.

Each of these is a wrapper around an internal function that takes as first
argument a (miPaintedSet *).  A miPaintedSet struct is a structure that is
used by Joel McCormack's span-merging module to implement the
`touch-each-pixel-once' rule.  See mi_spans.c and mi_spans.h. */

#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"

#define MI_SETUP_PAINTED_SET(paintedSet, pGC) \
{\
}

#define MI_TEAR_DOWN_PAINTED_SET(paintedSet) \
{\
  miUniquifyPaintedSet (paintedSet); \
}

void
#ifdef _HAVE_PROTOS
miDrawPoints (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
#else
miDrawPoints (paintedSet, pGC, mode, npt, pPts)
     miPaintedSet *paintedSet;
     const miGC *pGC;		/* unused */
     miCoordMode mode;		/* mode = Origin or Previous */
     int npt;
     const miPoint *pPts;
#endif
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miDrawPoints_internal (paintedSet, pGC, mode, npt, pPts);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

void
#ifdef _HAVE_PROTOS
miDrawLines (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
#else
miDrawLines (paintedSet, pGC, mode, npt, pPts)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     miCoordMode mode;
     int npt;
     const miPoint *pPts;
#endif
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miDrawLines_internal (paintedSet, pGC, mode, npt, pPts);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

void
#ifdef _HAVE_PROTOS
miFillPolygon (miPaintedSet *paintedSet, const miGC *pGC, miPolygonShape shape, miCoordMode mode, int count, const miPoint *pPts)
#else
miFillPolygon (paintedSet, pGC, shape, mode, count, pPts)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     miPolygonShape shape;
     miCoordMode mode;
     int count;
     const miPoint *pPts;
#endif
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miFillPolygon_internal (paintedSet, pGC, shape, mode, count, pPts);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

void
#ifdef _HAVE_PROTOS
miDrawRectangles (miPaintedSet *paintedSet, const miGC *pGC, int nrects, const miRectangle *prectInit)
#else
miDrawRectangles (paintedSet, pGC, nrects, prectInit)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int nrects; 
     const miRectangle *prectInit;
#endif
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC);
  miDrawRectangles_internal (paintedSet, pGC, nrects, prectInit);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

void
#ifdef _HAVE_PROTOS
miFillRectangles (miPaintedSet *paintedSet, const miGC *pGC, int nrectFill, const miRectangle *prectInit)
#else
miFillRectangles (paintedSet, pGC, nrectFill, prectInit)
     miPaintedSet *paintedSet;
     const miGC *pGC;		/* unused */
     int nrectFill; 
     const miRectangle *prectInit;
#endif
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC);
  miFillRectangles_internal (paintedSet, pGC, nrectFill, prectInit);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

#ifndef NO_NONREENTRANT_POLYARC_SUPPORT
void
#ifdef _HAVE_PROTOS
miDrawArcs (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs)
#else
miDrawArcs (paintedSet, pGC, narcs, parcs)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int narcs;
     const miArc *parcs;
#endif
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miDrawArcs_internal (paintedSet, pGC, narcs, parcs);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}
#endif /* not NO_NONREENTRANT_POLYARC_SUPPORT */

void
#ifdef _HAVE_PROTOS
miFillArcs (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs)
#else
miFillArcs (paintedSet, pGC, narcs, parcs)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int narcs;
     const miArc *parcs;
#endif
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miFillArcs_internal (paintedSet, pGC, narcs, parcs);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

void
#ifdef _HAVE_PROTOS
miDrawArcs_r (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs, miEllipseCache *ellipseCache)
#else
miDrawArcs_r (paintedSet, pGC, narcs, parcs, ellipseCache)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int narcs;
     const miArc *parcs;
     miEllipseCache *ellipseCache; /* pointer to ellipse data cache */
#endif
{
  MI_SETUP_PAINTED_SET(paintedSet, pGC)
  miDrawArcs_r_internal (paintedSet, pGC, narcs, parcs, ellipseCache);
  MI_TEAR_DOWN_PAINTED_SET(paintedSet)
}

/**********************************************************************/
/* Further wrappers that should really be moved to other file(s). */
/**********************************************************************/

void
#ifdef _HAVE_PROTOS
miDrawArcs_r_internal (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs, miEllipseCache *ellipseCache)
#else
miDrawArcs_r_internal (paintedSet, pGC, narcs, parcs, ellipseCache)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int narcs;
     const miArc *parcs;
     miEllipseCache *ellipseCache; /* pointer to ellipse data cache */
#endif
{
  if (pGC->lineWidth == 0)
    /* use Bresenham algorithm */
    miZeroPolyArc_r (paintedSet, pGC, narcs, parcs, ellipseCache);
  else
    miPolyArc_r (paintedSet, pGC, narcs, parcs, ellipseCache);
}

#ifndef NO_NONREENTRANT_POLYARC_SUPPORT
void
#ifdef _HAVE_PROTOS
miDrawArcs_internal (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs)
#else
miDrawArcs_internal (paintedSet, pGC, narcs, parcs)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int narcs;
     const miArc *parcs;
#endif
{
  if (pGC->lineWidth == 0)
    /* use Bresenham algorithm */
    miZeroPolyArc (paintedSet, pGC, narcs, parcs);
  else
    miPolyArc (paintedSet, pGC, narcs, parcs);
}
#endif /* not NO_NONREENTRANT_POLYARC_SUPPORT */

void
#ifdef _HAVE_PROTOS
miDrawLines_internal (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
#else
miDrawLines_internal (paintedSet, pGC, mode, npt, pPts)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     miCoordMode mode;
     int npt;
     const miPoint *pPts;
#endif
{
  if (pGC->lineWidth == 0)
    {
    /* use Bresenham algorithm */
      if (pGC->lineStyle == (int)MI_LINE_SOLID)
	miZeroLine (paintedSet, pGC, mode, npt, pPts);
      else
	miZeroDash (paintedSet, pGC, mode, npt, pPts);
    }
  else
    {
      if (pGC->lineStyle == (int)MI_LINE_SOLID)
	miWideLine (paintedSet, pGC, mode, npt, pPts);
      else
	miWideDash (paintedSet, pGC, mode, npt, pPts);
    }
}

void
#ifdef _HAVE_PROTOS
miDrawRectangles_internal (miPaintedSet *paintedSet, const miGC *pGC, int nrects, const miRectangle *prectInit)
#else
miDrawRectangles_internal (paintedSet, pGC, nrects, prectInit)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int nrects; 
     const miRectangle *prectInit;
#endif
{
  const miRectangle *pR = prectInit;
  miPoint rect[5];
  int i;

  for (i = 0; i < nrects; i++)
    {
      rect[0].x = pR->x;
      rect[0].y = pR->y;
      
      rect[1].x = pR->x + (int) pR->width;
      rect[1].y = rect[0].y;
      
      rect[2].x = rect[1].x;
      rect[2].y = pR->y + (int) pR->height;
      
      rect[3].x = rect[0].x;
      rect[3].y = rect[2].y;
      
      /* close the polyline */
      rect[4].x = rect[0].x;
      rect[4].y = rect[0].y;
      
      miDrawLines_internal (paintedSet, pGC, MI_COORD_MODE_ORIGIN, 5, rect);
      pR++;
    }
}
