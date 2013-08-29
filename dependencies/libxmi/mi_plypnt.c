#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"

/* This routine paints a set of points.  All painting goes through the
   low-level MI_PAINT_SPANS() macro. */

void
#ifdef _HAVE_PROTOS
miDrawPoints_internal (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
#else
miDrawPoints_internal (paintedSet, pGC, mode, npt, pPts)
     miPaintedSet *paintedSet;
     const miGC *pGC;		/* unused */
     miCoordMode mode;		/* mode = Origin or Previous */
     int npt;
     const miPoint *pPts;
#endif
{
  unsigned int	*pwidthInit, *pwidth;
  int		i;
  miPoint 	*ppt = (miPoint *)NULL;

  /* ensure we have >=1 points */
  if (npt <= 0)
    return;

  ppt = (miPoint *)mi_xmalloc (npt * sizeof(miPoint));
  if (mode == MI_COORD_MODE_PREVIOUS)
    /* convert from relative to absolute coordinates */
    {
      ppt[0] = pPts[0];
      for (i = 1; i < npt; i++)
	{
	  ppt[i].x = ppt[i-1].x + pPts[i].x;
	  ppt[i].y = ppt[i-1].y + pPts[i].y;	  
	}
    }
  else
    /* just copy */
    {
      for (i = 0; i < npt; i++)
	ppt[i] = pPts[i];
    }

  pwidthInit = (unsigned int *)mi_xmalloc (npt * sizeof(unsigned int));
  pwidth = pwidthInit;
  for (i = 0; i < npt; i++)
    *pwidth++ = 1;

  if (npt > 1)
    miQuickSortSpansY (ppt, pwidthInit, npt);
  MI_PAINT_SPANS(paintedSet, pGC->pixels[1], npt, ppt, pwidthInit)
}

