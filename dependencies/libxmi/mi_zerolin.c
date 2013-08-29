/* This module contains the miZeroLine() and miZeroDash() functions.  They
   a rasterize single-pixel (i.e., `zero-width') Bresenham polyline, either
   solid or dashed.

   The cap mode and join mode in the graphics context are ignored, except
   that if the cap mode is MI_CAP_NOT_LAST, the final pixel of the polyline
   is omitted.

   All painting goes through the low-level MI_PAINT_SPANS() and
   MI_COPY_AND_PAINT_SPANS() macros. */

/* Historical note: this is a merger of MI code from X11, written by Ken
   Whaley, with low-level X11 CFB (color frame-buffer) code, of unknown
   authorship.  (I wrote Ken, and he remembers writing the monochrome frame
   buffer code, but not the color code.)  That's because the X11 MI code
   included miZeroLine(), but not miZeroDash().  I based the latter on the
   CFB routines.  I needed to hack them extensively so that they would
   generate a list of spans to paint.  I also removed clipping code.  --rsm */

#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"
#include "mi_line.h"

/* Comment on drawing solid lines (from Ken Whaley):

   NON-SLOPED LINES

   Horizontal lines are always drawn left to right; we have to move the
   endpoints right by one after they're swapped.  Vertical lines are always
   drawn top to bottom (y-increasing).  This requires adding one to the
   y-coordinate of each endpoint after swapping. */

/* forward references */
static void cfbBresD ____P((miPaintedSet *paintedSet, const miGC *pGC, int *pdashNum, int *pdashIndex, const unsigned int *pDash, int numInDashList, int *pdashOffset, bool isDoubleDash, int signdx, int signdy, int axis, int x1, int y1, int e, int e1, int e2, int len));
static void cfbBresS ____P((miPaintedSet *paintedSet, const miGC *pGC, int signdx, int signdy, int axis, int x1, int y1, int e, int e1, int e2, int len));
static void cfbHorzS ____P((miPaintedSet *paintedSet, const miGC *pGC, int x1, int y1, int len));
static void cfbVertS ____P((miPaintedSet *paintedSet, const miGC *pGC, int x1, int y1, int len));


/* Macro for painting a single point.  Used for ending line segments. */

#define MI_PAINT_POINT(paintedSet, pixel, xx, yy) \
{\
  miPoint *point;\
  unsigned int *width;\
  point = (miPoint *)mi_xmalloc(sizeof(miPoint));\
  width = (unsigned int *)mi_xmalloc(sizeof(unsigned int));\
  *width = 1;\
  point->x = xx;\
  point->y = yy;\
  MI_PAINT_SPANS(paintedSet, pixel, 1, point, width)\
}

/* Macro for generating a list of spans, used when the successive points on
   a Bresenham line are generated.  Assumes the availability of working
   storage, accessed via pointers `ppt' and `pwidths', and also variables
   numSpans (initted to 0), firstspan (initted to true), and ycurr.  The
   arrays should be at least as large as the longest generated Bresenham
   line segment. */
   
#define MI_ADD_POINT(xx, yy, ppt, pwidth, numSpans, ycurr, firstspan, signdy) \
{\
    if (!firstspan && yy == ycurr)\
    {\
      int xdelta = xx - ppt->x;\
      if (xdelta < 0)\
          {\
	    (*pwidth) -= xdelta;\
	    ppt->x = xx;\
          }\
      else if (xdelta > 0)\
          { \
            unsigned int widthcurr = *pwidth; \
            (*pwidth) = UMAX(widthcurr, (unsigned int)(1 + xdelta));\
          } \
    }\
    else\
    {\
       if (!firstspan)\
         {\
	    ppt += signdy;\
	    pwidth += signdy;\
         }\
        else\
          firstspan = false;\
	ppt->x = xx;\
	ppt->y = yy;\
	*pwidth = 1;\
	ycurr = yy;\
        ++numSpans;\
    }\
}

/*
 * Draw a solid Bresenham polyline, i.e. a `zero-width' solid polyline,
 * in paint type #1.
 */
void
#ifdef _HAVE_PROTOS
miZeroLine (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
#else
miZeroLine (paintedSet, pGC, mode, npt, pPts)
     miPaintedSet *paintedSet;
     const miGC	*pGC;
     miCoordMode mode;		/* Origin or Previous */
     int npt;			/* number of points */
     const miPoint *pPts;	/* point array */
#endif
{
  const miPoint *ppt;	/* pointer to point within array */
  /* temporaries */
  int xstart, ystart;
  int x1, x2;
  int y1, y2;

  /* ensure we have >=1 points */
  if (npt <= 0)
    return;

  /* loop through points, drawing a solid Bresenham segment for each line
     segment */
  ppt = pPts;
  xstart = ppt->x;
  ystart = ppt->y;
  x2 = xstart;
  y2 = ystart;
  while (--npt)
    {
      x1 = x2;
      y1 = y2;
      ++ppt;

      x2 = ppt->x;
      y2 = ppt->y;
      if (mode == MI_COORD_MODE_PREVIOUS)
	/* convert from relative coordinates */
	{
	  x2 += x1;
	  y2 += y1;
	}

      if (x1 == x2)  /* vertical line */
	{
	  if (y1 > y2)
	    /* make line go top to bottom, keeping endpoint semantics */
	    {
	      int tmp;

	      tmp = y2;
	      y2 = y1 + 1;
	      y1 = tmp + 1;
	    }

	  /* draw line */
	  if (y1 != y2)
	    cfbVertS (paintedSet, pGC, x1, y1, y2 - y1);

	  /* restore final point */
	  y2 = ppt->y;
	}

      else if (y1 == y2)  /* horizontal line */
	{
	  if (x1 > x2)
	    /* force line from left to right, keeping endpoint semantics */
	    {
	      int tmp;

	      tmp = x2;
	      x2 = x1 + 1;
	      x1 = tmp + 1;
	    }
	  
	  /* draw line */
	  if (x1 != x2)
	    cfbHorzS (paintedSet, pGC, x1, y1, x2 - x1);
	  /* restore final point */
	  x2 = ppt->x;
	}

      else	/* sloped line */
	{
	  int adx;			/* abs values of dx and dy */
	  int ady;
	  int signdx;			/* sign of dx and dy */
	  int signdy;
	  int e, e1, e2;		/* Bresenham error and increments */
	  int axis;			/* major axis */
	  int len;			/* length of segment */

	  AbsDeltaAndSign(x2, x1, adx, signdx);
	  AbsDeltaAndSign(y2, y1, ady, signdy);
	  if (adx > ady)
	    {
	      axis = X_AXIS;
	      e1 = ady << 1;
	      e2 = e1 - (adx << 1);
	      e = e1 - adx;
	      FIXUP_X_MAJOR_ERROR(e, signdx, signdy);
 	    }
	  else
	    {
	      axis = Y_AXIS;
	      e1 = adx << 1;
	      e2 = e1 - (ady << 1);
	      e = e1 - ady;
	      FIXUP_Y_MAJOR_ERROR(e, signdx, signdy);
	    }

	  /* we have Bresenham parameters and two points, so all we need to
	     do now is draw */
	  if (axis == X_AXIS)
	    len = adx;
	  else
	    len = ady;
	  cfbBresS (paintedSet, pGC,
		    signdx, signdy, axis, x1, y1, 
		    e, e1, e2, len);
	} /* sloped line */
    } /* while (--npt) */
  
  /* Paint the last point if the end style isn't CapNotLast.  (I.e. assume
     that a round/butt/projecting/triangular cap that is one pixel wide is
     the same as the single pixel of the endpoint.) */
  if (pGC->capStyle != (int)MI_CAP_NOT_LAST
      &&
      (xstart != x2 || ystart != y2 || ppt == pPts + 1))
    MI_PAINT_POINT(paintedSet, pGC->pixels[1], x2, y2)
}


/*
 * Draw a dashed Bresenham polyline, i.e. a `zero-width' dashed polyline.
 */
void
#ifdef _HAVE_PROTOS
miZeroDash (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
#else
miZeroDash (paintedSet, pGC, mode, npt, pPts)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     miCoordMode mode;		/* Origin or Previous */
     int npt;			/* number of points */
     const miPoint *pPts;	/* point array */
#endif
{
  const miPoint *ppt;	/* pointer to current point */
  /* temporaries */
  int xstart, ystart;
  int x1, x2, y1, y2;

  const unsigned int    *pDash;
  int dashNum, dashIndex;
  int dashOffset;
  int numInDashList;
  bool isDoubleDash;

  /* ensure we have >=1 points */
  if (npt <= 0)
    return;

  /* perform initial offsetting into the dash array; compute dash values */
  pDash = pGC->dash;
  numInDashList = pGC->numInDashList;
  isDoubleDash = (pGC->lineStyle == (int)MI_LINE_DOUBLE_DASH ? true : false);
  dashNum = 0;
  dashIndex = 0;
  dashOffset = 0;
  miStepDash (pGC->dashOffset, &dashNum, &dashIndex, 
	      pDash, numInDashList, &dashOffset);

  /* loop through points, drawing a dashed Bresenham segment for each line
     segment of nonzero length */
  ppt = pPts;
  xstart = ppt->x;
  ystart = ppt->y;
  x2 = xstart;
  y2 = ystart;
  while (--npt)
    {
      x1 = x2;
      y1 = y2;
      ++ppt;

      x2 = ppt->x;
      y2 = ppt->y;
      if (mode == MI_COORD_MODE_PREVIOUS)
	/* convert from relative coordinates */
	{
	  x2 += x1;
	  y2 += y1;
	}
      
      /* use Bresenham algorithm for sloped lines (no special treatment for
	 horizontal or vertical lines, unlike the undashed case) */
      {
	int adx;		/* abs values of dx and dy */
	int ady;
	int signdx;		/* sign of dx and dy */
	int signdy;
	int e, e1, e2;		/* Bresenham error and increments */
	int axis;		/* major axis */
	int len;

	AbsDeltaAndSign(x2, x1, adx, signdx);
	AbsDeltaAndSign(y2, y1, ady, signdy);
	if (adx > ady)
	  {
	    axis = X_AXIS;
	    e1 = ady << 1;
	    e2 = e1 - (adx << 1);
	    e = e1 - adx;
	    len = adx;
	    FIXUP_X_MAJOR_ERROR(e, signdx, signdy);
	  }
	else
	  {
	    axis = Y_AXIS;
	    e1 = adx << 1;
	    e2 = e1 - (ady << 1);
	    e = e1 - ady;
	    len = ady;
	    FIXUP_Y_MAJOR_ERROR(e, signdx, signdy);
	  }
	
	/* we have Bresenham parameters and two points, so all we need to
	   do now is draw (updating dashNum, dashIndex and dashOffset) */
	cfbBresD (paintedSet, pGC,
		  &dashNum, &dashIndex, pDash, numInDashList,
		  &dashOffset, isDoubleDash,
		  signdx, signdy, axis, x1, y1,
		  e, e1, e2, len);
      }
    } /* while (nline--) */

  /* paint the last point if the end style isn't CapNotLast.
     (Assume that a projecting, butt, or round cap that is one
     pixel wide is the same as the single pixel of the endpoint.) */
  if (pGC->capStyle != (int)MI_CAP_NOT_LAST
      &&
      (xstart != x2 || ystart != y2 || ppt == pPts + 1))
    {
      if (dashNum & 1)
	{
	  /* background dash */
	  if (isDoubleDash)
	    /* paint, in paint type #0 */
	    MI_PAINT_POINT(paintedSet, pGC->pixels[0], x2, y2);
	}
      else
	/* foreground dash */
	{
	  /* use a paint type that cycles through 1..(numPixels-1) */
	  int numPixels = pGC->numPixels;
	  int paintType = 1 + ((dashNum / 2) % (numPixels - 1));
	  
	  MI_PAINT_POINT(paintedSet, pGC->pixels[paintType], x2, y2);
	}
    }
}


/* Internal: draw solid Bresenham line segment, in paint type #1.  Called
   by miZeroLine().  Endpoint semantics are used, i.e. we paint only len
   pixels (i.e. |dx| or |dy| pixels), not including the endpoint. */
static void
#ifdef _HAVE_PROTOS
cfbBresS (miPaintedSet *paintedSet, const miGC *pGC, int signdx, int signdy, int axis, int x1, int y1, int e, int e1, int e2, int len)
#else
cfbBresS (paintedSet, pGC, signdx, signdy, axis, x1, y1, e, e1, e2, len)
     miPaintedSet   *paintedSet;
     const miGC	    *pGC;
     int    	    signdx;
     int	    signdy;		/* signs of directions */
     int	    axis;		/* major axis (Y_AXIS or X_AXIS) */
     int	    x1, y1;		/* initial point */
     int    	    e;			/* error accumulator */
     int    	    e1;			/* Bresenham increments */
     int	    e2;
     int	    len;		/* length of line in pixels */
#endif
{
  miPoint *pptInit, *pptLast;
  unsigned int *pwidthInit, *pwidthLast;
  int x, y;
  int e3;

  /* variables in span generation code, i.e. in MI_ADD_POINT() */
  int numSpans;
  int ycurr;
  miPoint *ppt;
  unsigned int *pwidth;
  bool firstspan;

  if (len == 0)
    return;

  /* set up work arrays */
  pptInit = (miPoint *)mi_xmalloc(len * sizeof(miPoint));
  pwidthInit = (unsigned int *)mi_xmalloc(len * sizeof(unsigned int));
  pptLast = pptInit + (len - 1);
  pwidthLast = pwidthInit + (len - 1);

  /* reset variables used in MI_ADD_POINT() */
  numSpans = 0;
  ycurr = 0;
  firstspan = true;
  if (signdy >= 0)
    {
      ppt  = pptInit;
      pwidth = pwidthInit;
    }
  else
    {
      ppt  = pptLast;
      pwidth = pwidthLast;
    }
      
  e3 = e2 - e1;
  e = e - e1;			/* make looping easier */

#define BresStep(minor,major) \
	{if ((e += e1) >= 0) { e += e3; minor; } major;}

#define Loop_x(counter,store) while (counter--) \
	{store; BresStep(y+=signdy,x+=signdx) }
#define Loop_y(counter,store) while (counter--) \
	{store; BresStep(x+=signdx, y+=signdy) }

  /* point to first point, and generate len pixels */
  x = x1;
  y = y1;

  switch (axis)
    {
    case X_AXIS:
    default:
        Loop_x(len, MI_ADD_POINT(x, y, ppt, pwidth, numSpans, ycurr, firstspan, signdy))
	break;
    case Y_AXIS:
        Loop_y(len, MI_ADD_POINT(x, y, ppt, pwidth, numSpans, ycurr, firstspan, signdy))
	break;
    }

  if (numSpans > 0)
    {
      if (signdy < 0)
	/* spans are offset, so shift downward  */
	{
	  miPoint *ppt_src = pptLast - (numSpans - 1);
	  miPoint *ppt_dst = pptInit;
	  unsigned int *pwidth_src = pwidthLast - (numSpans - 1);
	  unsigned int *pwidth_dst = pwidthInit;
	  int count = numSpans;

	  while (count--)
	    {
	      *ppt_dst++ = *ppt_src++;
	      *pwidth_dst++ = *pwidth_src++;
	    }
	}

      MI_PAINT_SPANS(paintedSet, pGC->pixels[1], numSpans, pptInit, pwidthInit)
    }
}

/* Internal: draw dashed Bresenham line segment. Called by miZeroDash().
   Endpoint semantics are used. */
static void
#ifdef _HAVE_PROTOS
cfbBresD (miPaintedSet *paintedSet, const miGC *pGC, int *pdashNum, int *pdashIndex, const unsigned int *pDash, int numInDashList, int *pdashOffset, bool isDoubleDash, int signdx, int signdy, int axis, int x1, int y1, int e, int e1, int e2, int len)
#else
cfbBresD (paintedSet, pGC, pdashNum, pdashIndex, pDash, numInDashList, pdashOffset, isDoubleDash, signdx, signdy, axis, x1, y1, e, e1, e2, len)
     miPaintedSet   *paintedSet;
     const miGC	    *pGC;
     int	    *pdashNum;		/* absolute dash number */
     int	    *pdashIndex;	/* index into dash array */
     const unsigned int   *pDash;	/* dash array */
     int	    numInDashList;	/* length of dash array */
     int	    *pdashOffset;	/* offset into current dash */
     bool	    isDoubleDash;
     int            signdx, signdy;	/* signs of directions */
     int	    axis;		/* major axis (Y_AXIS or X_AXIS) */
     int	    x1, y1;		/* initial point */
     int	    e;			/* error accumulator */
     int    	    e1;			/* Bresenham increments */
     int	    e2;
     int	    len;		/* length of line in pixels */
#endif
{
  miPoint *pptInit_fg, *pptInit_bg = (miPoint *)NULL;
  miPoint *pptLast_fg, *pptLast_bg = (miPoint *)NULL;
  unsigned int *pwidthInit_fg, *pwidthInit_bg = (unsigned int *)NULL;
  unsigned int *pwidthLast_fg, *pwidthLast_bg = (unsigned int *)NULL;
  int		x, y;
  int 		e3;
  int		dashNum, dashIndex;
  int		dashOffset;
  int		dashRemaining;
  int		thisDash;

  /* variables in span generation code, i.e. in MI_ADD_POINT() */
  int numSpans_fg, numSpans_bg = 0;
  int ycurr_fg, ycurr_bg = 0;
  miPoint *ppt_fg, *ppt_bg = (miPoint *)NULL;
  unsigned int *pwidth_fg, *pwidth_bg = (unsigned int *)NULL;
  bool firstspan_fg, firstspan_bg = false;

  /* set up work arrays */
  pptInit_fg = (miPoint *)mi_xmalloc(len * sizeof(miPoint));
  pwidthInit_fg = (unsigned int *)mi_xmalloc(len * sizeof(unsigned int));
  pptLast_fg = pptInit_fg + (len - 1);
  pwidthLast_fg = pwidthInit_fg + (len - 1);
  if (isDoubleDash)
    {
      pptInit_bg = (miPoint *)mi_xmalloc(len * sizeof(miPoint));
      pwidthInit_bg = (unsigned int *)mi_xmalloc(len * sizeof(unsigned int));
      pptLast_bg = pptInit_bg + (len - 1);
      pwidthLast_bg = pwidthInit_bg + (len - 1);
    }

  dashNum = *pdashNum;		/* absolute number of current dash */
  dashIndex = *pdashIndex;	/* index of current dash */
  dashOffset = *pdashOffset;	/* offset into current dash */
  dashRemaining = (int)(pDash[dashIndex]) - dashOffset;	/* how much is left */
  if (len <= (thisDash = dashRemaining))
    /* line segment will be solid, not dashed */
    {
      thisDash = len;
      dashRemaining -= len;
    }

#define NextDash {\
    dashNum++; \
    dashIndex++; \
    if (dashIndex == numInDashList) \
	dashIndex = 0; \
    dashRemaining = (int)(pDash[dashIndex]); \
    if ((thisDash = dashRemaining) >= len) \
    { \
	dashRemaining -= len; \
	thisDash = len; \
    } \
}

  e3 = e2-e1;
  e = e - e1;			/* make looping easier */

  /* point to first point */
  x = x1;
  y = y1;

  /* loop, generating dashes (in the absence of dashing, would
     generate len pixels in all) */
  for ( ; ; )
    { 
      len -= thisDash;

      /* reset variables used in MI_ADD_POINT() */
      numSpans_fg = 0;
      ycurr_fg = 0;
      firstspan_fg = true;
      if (signdy >= 0)
	{
	  ppt_fg = pptInit_fg;
	  pwidth_fg = pwidthInit_fg;
	}
      else
	{
	  ppt_fg  = pptLast_fg;
	  pwidth_fg = pwidthLast_fg;
	}
      if (isDoubleDash)
	{
	  numSpans_bg = 0;
	  ycurr_bg = 0;
	  firstspan_bg = true;
	  ppt_bg  = pptInit_bg;
	  pwidth_bg = pwidthInit_bg;
	  if (signdy >= 0)
	    {
	      ppt_bg = pptInit_bg;
	      pwidth_bg = pwidthInit_bg;
	    }
	  else
	    {
	      ppt_bg  = pptLast_bg;
	      pwidth_bg = pwidthLast_bg;
	    }
	}

      switch (axis)
	{
	case X_AXIS:
	default:
	  if (dashIndex & 1) 
	    {
	      if (isDoubleDash) 
		{
		  /* create background dash */
		  Loop_x(thisDash, MI_ADD_POINT(x, y, ppt_bg, pwidth_bg, numSpans_bg, ycurr_bg, firstspan_bg, signdy))
		}
	      else 
		/* not double dashing; no background dash */
	        Loop_x(thisDash, ;);
	    } 
	  else
	    /* create foreground dash */
	    Loop_x(thisDash, MI_ADD_POINT(x, y, ppt_fg, pwidth_fg, numSpans_fg, ycurr_fg, firstspan_fg, signdy))
	  break;
	case Y_AXIS:
	  if (dashIndex & 1) 
	    {
	      if (isDoubleDash) 
		{
		  /* create background dash */
		  Loop_y(thisDash, MI_ADD_POINT(x, y, ppt_bg, pwidth_bg, numSpans_bg, ycurr_bg, firstspan_bg, signdy))
		}
	      else 
		/* not double dashing; no background dash */
	        Loop_y(thisDash, ;);
	    } 
	  else
	    /* create foreground dash */
	    Loop_y(thisDash, MI_ADD_POINT(x, y, ppt_fg, pwidth_fg, numSpans_fg, ycurr_fg, firstspan_fg, signdy))
	  break;
	} /* end switch */
      
      if (numSpans_fg > 0)
	/* have a foreground dash to paint */
	{
	  miPoint *pptStart_fg;
	  unsigned int *pwidthStart_fg;
	  
	  if (signdy >= 0)
	    {
	      pptStart_fg = pptInit_fg;
	      pwidthStart_fg = pwidthInit_fg;
	    }
	  else
	    {
	      pptStart_fg = pptLast_fg - (numSpans_fg - 1);
	      pwidthStart_fg = pwidthLast_fg - (numSpans_fg - 1);
	    }
	  /* for foreground dash, use a paint type that cycles through
	     1..(numPixels-1) */
	  {
	    int numPixels = pGC->numPixels;
	    int paintType = 1 + ((dashNum / 2) % (numPixels - 1));

	    MI_COPY_AND_PAINT_SPANS(paintedSet, pGC->pixels[paintType], numSpans_fg, pptStart_fg, pwidthStart_fg)
	  }
	}

      if (isDoubleDash && numSpans_bg > 0)
	/* have a background dash to paint */
	{
	  miPoint *pptStart_bg;
	  unsigned int *pwidthStart_bg;
	  
	  if (signdy >= 0)
	    {
	      pptStart_bg = pptInit_bg;
	      pwidthStart_bg = pwidthInit_bg;
	    }
	  else
	    {
	      pptStart_bg = pptLast_bg - (numSpans_bg - 1);
	      pwidthStart_bg = pwidthLast_bg - (numSpans_bg - 1);
	    }
	  /* for background dash, use paint type #0 */
	  MI_COPY_AND_PAINT_SPANS(paintedSet, pGC->pixels[0], numSpans_bg, pptStart_bg, pwidthStart_bg)
	}

      if (len == 0)
	break;			/* break out of dash-generating loop */

      NextDash	    /* update dashNum, dashIndex, thisDash, dashRemaining */

    } /* end infinite dash-generating loop */

  *pdashNum = dashNum;
  *pdashIndex = dashIndex;
  *pdashOffset = (int)(pDash[dashIndex]) - dashRemaining;

  free (pwidthInit_fg);
  free (pptInit_fg);
  if (isDoubleDash)
    {
      free (pwidthInit_bg);
      free (pptInit_bg);
    }
}

/* Internal: draw horizontal zero-width solid line segment, in paint type #1.
   Called by miZeroLine().
   Called with len >= 1, and len=x2-x1.  Endpoint semantics
   are used, so we paint only len pixels, i.e. x1..x2-1. */
static void
#ifdef _HAVE_PROTOS
cfbHorzS (miPaintedSet *paintedSet, const miGC *pGC, int x1, int y1, int len)
#else
cfbHorzS (paintedSet, pGC, x1, y1, len)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int	x1, y1;		/* initial point */ 
     int	len;		/* length of line */
#endif
{
  miPoint *ppt;
  unsigned int *pwidth;

  ppt = (miPoint *)mi_xmalloc(sizeof(miPoint));
  pwidth = (unsigned int *)mi_xmalloc(sizeof(unsigned int));

  ppt->x = x1;
  ppt->y = y1;
  *pwidth = (unsigned int)len;

  MI_PAINT_SPANS(paintedSet, pGC->pixels[1], 1, ppt, pwidth)
}

/* Internal: draw vertical zero-width solid line segment, in paint type #1.
   Called by miZeroLine().
   Called with len >= 1, and len=y2-y1.  Endpoint semantics
   are used, so we paint only len pixels, i.e. y1..y2-1. */
static void
#ifdef _HAVE_PROTOS
cfbVertS (miPaintedSet *paintedSet, const miGC *pGC, int x1, int y1, int len)
#else
cfbVertS (paintedSet, pGC, x1, y1, len)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int	x1, y1;		/* initial point */
     int	len;		/* length of line */
#endif
{
  miPoint *ppt, *pptInit;
  unsigned int *pwidth, *pwidthInit;
  int i;

  ppt = pptInit = (miPoint *)mi_xmalloc(len * sizeof(miPoint));
  pwidth = pwidthInit = (unsigned int *)mi_xmalloc(len * sizeof(unsigned int));

  for (i = 0; i < len; i++)
    {
      ppt->x = x1;
      ppt->y = y1 + i;
      ppt++;
      *pwidth++ = (unsigned int)1;
    }

  MI_PAINT_SPANS(paintedSet, pGC->pixels[1], len, pptInit, pwidthInit)
}
