/* Original author unknown (Bob Scheifler?).  
   Hacked by Robert S. Maier, 1998-1999. */

/* This module provides the function miFillArcs_internal(), which fills a
   poly-arc.  All painting goes through the low-level MI_PAINT_SPANS()
   macro. */

#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"
#include "mi_fllarc.h"

#define QUADRANT (90 * 64)
#define HALFCIRCLE (180 * 64)
#define QUADRANT3 (270 * 64)

/* trig functions, angle specified in 1/64 degrees */
#define Dsin(d)	sin((double)d*(M_PI/11520.0))
#define Dcos(d)	cos((double)d*(M_PI/11520.0))

/* internal functions that do painting of pixels */
static void miFillArcSliceD ____P((miPaintedSet *paintedSet, const miGC *pGC, const miArc *arc));
static void miFillArcSliceI ____P((miPaintedSet *paintedSet, const miGC *pGC, const miArc *arc));
static void miFillEllipseD ____P((miPaintedSet *paintedSet, const miGC *pGC, const miArc *arc));
static void miFillEllipseI ____P((miPaintedSet *paintedSet, const miGC *pGC, const miArc *arc));

/* internal functions that don't do painting of pixels */
static void miEllipseAngleToSlope ____P((int angle, unsigned int width, unsigned int height, int *dxp, int *dyp, double *d_dxp, double *d_dyp));
static void miFillArcDSetup ____P((const miArc *arc, miFillArcD *info));
static void miFillArcSetup ____P((const miArc *arc, miFillArc *info));
static void miFillArcSliceSetup ____P((const miGC *pGC, const miArc *arc, miArcSlice *slice));
static void miGetArcEdge ____P((const miArc *arc, miSliceEdge *edge, int k, bool top, bool left));
static void miGetPieEdge ____P((const miArc *arc, int angle, miSliceEdge *edge, bool top, bool left));


static void
#ifdef _HAVE_PROTOS
miFillArcSetup (const miArc *arc, miFillArc *info)
#else
miFillArcSetup (arc, info)
     const miArc *arc;
     miFillArc *info;
#endif
{
  info->y = arc->height >> 1;
  info->dy = arc->height & 1;
  info->yorg = arc->y + info->y;
  info->dx = arc->width & 1;
  info->xorg = arc->x + (int)(arc->width >> 1) + info->dx;
  info->dx = 1 - info->dx;

  if (arc->width == arc->height) /* circular arc */
    {
      /* (2x - 2xorg)^2 = d^2 - (2y - 2yorg)^2 */
      /* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
      info->ym = 8;
      info->xm = 8;
      info->yk = info->y << 3;
      if (!info->dx)
	{
	  info->xk = 0;
	  info->e = -1;
	}
      else
	{
	  info->y++;
	  info->yk += 4;
	  info->xk = -4;
	  info->e = - (info->y << 3);
	}
    }
  else				/* non-circular arc */
    {
      /* h^2 * (2x - 2xorg)^2 = w^2 * h^2 - w^2 * (2y - 2yorg)^2 */
      /* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
      info->ym = (arc->width * arc->width) << 3;
      info->xm = (arc->height * arc->height) << 3;
      info->yk = info->y * info->ym;
      if (!info->dy)
	info->yk -= info->ym >> 1;
      if (!info->dx)
	{
	  info->xk = 0;
	  info->e = - (info->xm >> 3);
	}
      else
	{
	  info->y++;
	  info->yk += info->ym;
	  info->xk = -(info->xm >> 1);
	  info->e = info->xk - info->yk;
	}
    }
}

static void
#ifdef _HAVE_PROTOS
miFillArcDSetup (const miArc *arc, miFillArcD *info)
#else
miFillArcDSetup (arc, info)
     const miArc *arc;
     miFillArcD *info;
#endif
{
  /* h^2 * (2x - 2xorg)^2 = w^2 * h^2 - w^2 * (2y - 2yorg)^2 */
  /* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
  info->y = arc->height >> 1;
  info->dy = arc->height & 1;
  info->yorg = arc->y + info->y;
  info->dx = arc->width & 1;
  info->xorg = arc->x + (int)(arc->width >> 1) + info->dx;
  info->dx = 1 - info->dx;
  info->ym = ((double)arc->width) * (arc->width * 8);
  info->xm = ((double)arc->height) * (arc->height * 8);
  info->yk = info->y * info->ym;
  if (!info->dy)
    info->yk -= (0.5 * info->ym);
  if (!info->dx)
    {
      info->xk = 0;
      info->e = - (0.125 * info->xm);
    }
  else
    {
      info->y++;
      info->yk += info->ym;
      info->xk = -0.5 * info->xm;
      info->e = info->xk - info->yk;
    }
}

static void
#ifdef _HAVE_PROTOS
miGetArcEdge (const miArc *arc, miSliceEdge *edge, int k, bool top, bool left)
#else
miGetArcEdge (arc, edge, k, top, left)
     const miArc *arc;
     miSliceEdge *edge;
     int k;
     bool top, left;
#endif
{
  int xady, y;

  y = arc->height >> 1;
  if ((arc->width & 1) == 0)
    y++;
  if (!top)
    {
      y = -y;
      if (arc->height & 1)
	y--;
    }
  xady = k + y * edge->dx;
  if (xady <= 0)
    edge->x = - ((-xady) / edge->dy + 1);
  else
    edge->x = (xady - 1) / edge->dy;
  edge->e = xady - edge->x * edge->dy;
  if ((top && (edge->dx < 0)) || (!top && (edge->dx > 0)))
    edge->e = edge->dy - edge->e + 1;
  if (left)
    edge->x++;
  edge->x += arc->x + (int)(arc->width >> 1);
  if (edge->dx > 0)
    {
      edge->deltax = 1;
      edge->stepx = edge->dx / edge->dy;
      edge->dx = edge->dx % edge->dy;
    }
  else
    {
      edge->deltax = -1;
      edge->stepx = - ((-edge->dx) / edge->dy);
      edge->dx = (-edge->dx) % edge->dy;
    }
  if (!top)
    {
      edge->deltax = -edge->deltax;
      edge->stepx = -edge->stepx;
    }
}

static void
#ifdef _HAVE_PROTOS
miEllipseAngleToSlope (int angle, unsigned int width, unsigned int height, int *dxp, int *dyp, double *d_dxp, double *d_dyp)
#else
miEllipseAngleToSlope (angle, width, height, dxp, dyp, d_dxp, d_dyp)
     int angle;
     unsigned int width, height;
     int *dxp, *dyp;
     double *d_dxp, *d_dyp;
#endif
{
  int     dx, dy;
  double  d_dx, d_dy, scale;
  bool    negative_dx, negative_dy;

  switch (angle) 
    {
    case 0:
      *dxp = -1;
      *dyp = 0;
      if (d_dxp) 
	{
	  *d_dxp = 0.5 * (int)width;
	  *d_dyp = 0;
	}
      break;
    case QUADRANT:
      *dxp = 0;
      *dyp = 1;
      if (d_dxp) 
	{
	  *d_dxp = 0;
	  *d_dyp = -0.5 * (int)height;
	}
      break;
    case HALFCIRCLE:
      *dxp = 1;
      *dyp = 0;
      if (d_dxp) 
	{
	  *d_dxp = -0.5 * (int)width;
	  *d_dyp = 0;
	}
      break;
    case QUADRANT3:
      *dxp = 0;
      *dyp = -1;
      if (d_dxp) 
	{
	  *d_dxp = 0;
	  *d_dyp = 0.5 * (int)height;
	}
      break;
    default:			/* angle is not 0, 90, 180, or 270 */
      d_dx = Dcos(angle) * (int)width;
      d_dy = Dsin(angle) * (int)height;
      if (d_dxp) 
	{
	  *d_dxp = 0.5 * d_dx;
	  *d_dyp = -0.5 * d_dy;
	}
      negative_dx = false;
      if (d_dx < 0.0)
	{
	  d_dx = -d_dx;
	  negative_dx = true;
	}
      negative_dy = false;
      if (d_dy < 0.0)
	{
	  d_dy = -d_dy;
	  negative_dy = true;
	}
      scale = d_dx;
      if (d_dy > d_dx)
	scale = d_dy;
      dx = IFLOOR((d_dx * 32768) / scale + 0.5);
      if (negative_dx)
	dx = -dx;
      *dxp = dx;
      dy = IFLOOR((d_dy * 32768) / scale + 0.5);
      if (negative_dy)
	dy = -dy;
      *dyp = dy;
      break;
    }
}

static void
#ifdef _HAVE_PROTOS
miGetPieEdge (const miArc *arc, int angle, miSliceEdge *edge, bool top, bool left)
#else
miGetPieEdge (arc, angle, edge, top, left)
     const miArc *arc;
     int angle;
     miSliceEdge *edge;
     bool top, left;
#endif
{
  int k;
  int dx, dy;
  /* why no signdx, signdy? */

  miEllipseAngleToSlope (angle, arc->width, arc->height, &dx, &dy, NULL, NULL);

  if (dy == 0)
    {
      edge->x = left ? INT_MIN : INT_MAX;
      edge->stepx = 0;
      edge->e = 0;
      edge->dx = -1;
      return;
    }
  if (dx == 0)
    {
      edge->x = arc->x + (int)(arc->width >> 1);
      if (left && (arc->width & 1))
	edge->x++;
      else if (!left && !(arc->width & 1))
	edge->x--;
      edge->stepx = 0;
      edge->e = 0;
      edge->dx = -1;
      return;
    }
  if (dy < 0) 
    {
      dx = -dx;
      dy = -dy;
    }
  k = (arc->height & 1) ? dx : 0;
  if (arc->width & 1)
    k += dy;
  edge->dx = dx << 1;
  edge->dy = dy << 1;
  miGetArcEdge (arc, edge, k, top, left);
}

static void
#ifdef _HAVE_PROTOS
miFillArcSliceSetup (const miGC *pGC, const miArc *arc, miArcSlice *slice)
#else
miFillArcSliceSetup (pGC, arc, slice)
     const miGC *pGC;
     const miArc *arc;
     miArcSlice *slice;
#endif
{
  int angle1, angle2;

  angle1 = arc->angle1;
  if (arc->angle2 < 0)
    {
      angle2 = angle1;
      angle1 += arc->angle2;
    }
  else
    angle2 = angle1 + arc->angle2;
  while (angle1 < 0)
    angle1 += FULLCIRCLE;
  while (angle1 >= FULLCIRCLE)
    angle1 -= FULLCIRCLE;
  while (angle2 < 0)
    angle2 += FULLCIRCLE;
  while (angle2 >= FULLCIRCLE)
    angle2 -= FULLCIRCLE;
  slice->min_top_y = 0;
  slice->max_top_y = arc->height >> 1;
  slice->min_bot_y = 1 - (arc->height & 1);
  slice->max_bot_y = slice->max_top_y - 1;
  slice->flip_top = false;
  slice->flip_bot = false;

  switch (pGC->arcMode)
    {
    case MI_ARC_CHORD:		/* chord filling, not pie slice */
    default:
      {
	double w2, h2, x1, y1, x2, y2, dx, dy, scale;
	int signdx, signdy, y, k;
	bool isInt1 = true, isInt2 = true;

	w2 = 0.5 * (double)arc->width;
	h2 = 0.5 * (double)arc->height;
	if ((angle1 == 0) || (angle1 == HALFCIRCLE))
	  {
	    x1 = angle1 ? -w2 : w2;
	    y1 = 0.0;
	  }
	else if ((angle1 == QUADRANT) || (angle1 == QUADRANT3))
	  {
	    x1 = 0.0;
	    y1 = (angle1 == QUADRANT) ? h2 : -h2;
	  }
	else
	  {
	    isInt1 = false;
	    x1 = Dcos(angle1) * w2;
	    y1 = Dsin(angle1) * h2;
	  }
	if ((angle2 == 0) || (angle2 == HALFCIRCLE))
	  {
	    x2 = angle2 ? -w2 : w2;
	    y2 = 0.0;
	  }
	else if ((angle2 == QUADRANT) || (angle2 == QUADRANT3))
	  {
	    x2 = 0.0;
	    y2 = (angle2 == QUADRANT) ? h2 : -h2;
	  }
	else
	  {
	    isInt2 = false;
	    x2 = Dcos(angle2) * w2;
	    y2 = Dsin(angle2) * h2;
	  }
	dx = x2 - x1;
	dy = y2 - y1;
	if (arc->height & 1)
	  {
	    y1 -= 0.5;
	    y2 -= 0.5;
	  }
	if (arc->width & 1)
	  {
	    x1 += 0.5;
	    x2 += 0.5;
	  }
	if (dy < 0.0)
	  {
	    dy = -dy;
	    signdy = -1;
	  }
	else
	  signdy = 1;
	if (dx < 0.0)
	  {
	    dx = -dx;
	    signdx = -1;
	  }
	else
	  signdx = 1;
	if (isInt1 && isInt2)
	  {
	    slice->edge1.dx = (int)(dx * 2);
	    slice->edge1.dy = (int)(dy * 2);
	  }
	else
	  {
	    scale = (dx > dy) ? dx : dy;
	    slice->edge1.dx = IFLOOR((dx * 32768) / scale + .5);
	    slice->edge1.dy = IFLOOR((dy * 32768) / scale + .5);
	  }
	if (!slice->edge1.dy)
	  {
	    if (signdx < 0)
	      {
		y = IFLOOR(y1 + 1.0);
		if (y >= 0)
		  {
		    slice->min_top_y = y;
		    slice->min_bot_y = arc->height;
		  }
		else
		  {
		    slice->max_bot_y = -y - (arc->height & 1);
		  }
	      }
	    else
	      {
		y = IFLOOR(y1);
		if (y >= 0)
		  slice->max_top_y = y;
		else
		  {
		    slice->min_top_y = arc->height;
		    slice->min_bot_y = -y - (arc->height & 1);
		  }
	      }
	    slice->edge1_top = true;
	    slice->edge1.x = INT_MAX;
	    slice->edge1.stepx = 0;
	    slice->edge1.e = 0;
	    slice->edge1.dx = -1;
	    slice->edge2 = slice->edge1;
	    slice->edge2_top = false;
	  }
	else if (!slice->edge1.dx)
	  {
	    if (signdy < 0)
	      x1 -= 1.0;
	    slice->edge1.x = ICEIL(x1);
	    slice->edge1_top = (signdy < 0 ? true : false);
	    slice->edge1.x += arc->x + (int)(arc->width >> 1);
	    slice->edge1.stepx = 0;
	    slice->edge1.e = 0;
	    slice->edge1.dx = -1;
	    slice->edge2_top = (slice->edge1_top ? false : true);
	    slice->edge2 = slice->edge1;
	  }
	else
	  {
	    if (signdx < 0)
	      slice->edge1.dx = -slice->edge1.dx;
	    if (signdy < 0)
	      slice->edge1.dx = -slice->edge1.dx;
	    k = ICEIL(((x1 + x2) * slice->edge1.dy - (y1 + y2) * slice->edge1.dx) / 2.0);
	    slice->edge2.dx = slice->edge1.dx;
	    slice->edge2.dy = slice->edge1.dy;
	    slice->edge1_top = (signdy < 0 ? true : false);
	    slice->edge2_top = (slice->edge1_top ? false : true);
	    miGetArcEdge(arc, &slice->edge1, k,
			 slice->edge1_top, (slice->edge1_top ? false : true));
	    miGetArcEdge(arc, &slice->edge2, k,
			 slice->edge2_top, slice->edge2_top);
	  }
      }
      break;

    case MI_ARC_PIE_SLICE:		/* pie slice filling, not chord */
      slice->edge1_top = (angle1 < HALFCIRCLE ? true : false);
      slice->edge2_top = (angle2 <= HALFCIRCLE ? true : false);
      if ((angle2 == 0) || (angle1 == HALFCIRCLE))
	{
	  if (angle2 ? slice->edge2_top : slice->edge1_top)
	    slice->min_top_y = slice->min_bot_y;
	  else
	    slice->min_top_y = arc->height;
	  slice->min_bot_y = 0;
	}
      else if ((angle1 == 0) || (angle2 == HALFCIRCLE))
	{
	  slice->min_top_y = slice->min_bot_y;
	  if (angle1 ? slice->edge1_top : slice->edge2_top)
	    slice->min_bot_y = (int)arc->height;
	  else
	    slice->min_bot_y = 0;
	}
      else if (slice->edge1_top == slice->edge2_top)
	{
	  if (angle2 < angle1)
	    {
	      slice->flip_top = slice->edge1_top;
	      slice->flip_bot = (slice->edge1_top ? false : true);
	    }
	  else if (slice->edge1_top)
	    {
	      slice->min_top_y = 1;
	      slice->min_bot_y = (int)arc->height;
	    }
	  else
	    {
	      slice->min_bot_y = 0;
	      slice->min_top_y = (int)arc->height;
	    }
	}
      miGetPieEdge(arc, angle1, &slice->edge1,
		   slice->edge1_top, (slice->edge1_top ? false : true));
      miGetPieEdge(arc, angle2, &slice->edge2,
		   slice->edge2_top, slice->edge2_top);
      break;
    }
}

#define ADDSPANS(pts_upper, pts_lower, wids_upper, wids_lower, x, y, slw, e, xk, xorg, yorg, dy) \
  pts_upper->x = xorg - x; \
  pts_upper->y = yorg - y; \
  *wids_upper = (unsigned int)slw; \
  pts_upper++; \
  wids_upper++; \
  if (MIFILLARCLOWER(e, xk, y, dy, slw)) \
  { \
    pts_lower->x = xorg - x; \
    pts_lower->y = yorg + y + dy; \
    pts_lower--; \
    *wids_lower-- = (unsigned int)slw; \
  }

static void
#ifdef _HAVE_PROTOS
miFillEllipseI (miPaintedSet *paintedSet, const miGC *pGC, const miArc *arc)
#else
miFillEllipseI (paintedSet, pGC, arc)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     const miArc *arc;
#endif
{
  int x, y, e;
  int yk, xk, ym, xm, dx, dy, xorg, yorg;
  int slw;
  miFillArc info;
  miPoint *points_upper, *points_lower, *pts_upper, *pts_lower;
  unsigned int *widths_upper, *widths_lower, *wids_upper, *wids_lower;
  int numUpperSpans, numLowerSpans;

  points_upper = (miPoint *)mi_xmalloc (sizeof(miPoint) * arc->height);
  widths_upper = (unsigned int *)mi_xmalloc (sizeof(unsigned int) * arc->height);
  pts_upper = points_upper;
  wids_upper = widths_upper;
  points_lower = (miPoint *)mi_xmalloc (sizeof(miPoint) * arc->height);
  widths_lower = (unsigned int *)mi_xmalloc (sizeof(unsigned int) * arc->height);
  pts_lower = points_lower + (arc->height - 1);	/* begin at end, will decr */
  wids_lower = widths_lower + (arc->height - 1);

  miFillArcSetup(arc, &info);
  MIFILLARCSETUP(info, x, y, e, xk, xm, yk, ym, dx, dy, xorg, yorg);
  while (y > 0)
    /* add an upper and maybe a lower span (resp. growing downward, upward) */
    {
      MIFILLARCSTEP(x, y, e, xk, xm, yk, ym, dx, slw); /* y-- */
      ADDSPANS(pts_upper, pts_lower, wids_upper, wids_lower, x, y, slw, e, xk, xorg, yorg, dy);
    }

  numUpperSpans = pts_upper - points_upper;
  numLowerSpans = points_lower + (arc->height - 1) - pts_lower;
  if (numUpperSpans > 0)
    MI_PAINT_SPANS(paintedSet, pGC->pixels[1], numUpperSpans, points_upper, widths_upper)
  if (numLowerSpans > 0)
    MI_COPY_AND_PAINT_SPANS(paintedSet, pGC->pixels[1], numLowerSpans, pts_lower + 1, wids_lower + 1)
  free (points_lower);
  free (widths_lower);
}

static void
#ifdef _HAVE_PROTOS
miFillEllipseD (miPaintedSet *paintedSet, const miGC *pGC, const miArc *arc)
#else
miFillEllipseD (paintedSet, pGC, arc)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     const miArc *arc;
#endif
{
  int x, y;
  int xorg, yorg, dx, dy, slw;
  double e, yk, xk, ym, xm;
  miFillArcD info;
  miPoint *points_upper, *points_lower, *pts_upper, *pts_lower;
  unsigned int *widths_upper, *widths_lower, *wids_upper, *wids_lower;
  int numUpperSpans, numLowerSpans;

  points_upper = (miPoint *)mi_xmalloc (sizeof(miPoint) * arc->height);
  widths_upper = (unsigned int *)mi_xmalloc (sizeof(unsigned int) * arc->height);
  pts_upper = points_upper;
  wids_upper = widths_upper;
  points_lower = (miPoint *)mi_xmalloc (sizeof(miPoint) * arc->height);
  widths_lower = (unsigned int *)mi_xmalloc (sizeof(unsigned int) * arc->height);
  pts_lower = points_lower + (arc->height - 1);	/* begin at end, will decr */
  wids_lower = widths_lower + (arc->height - 1);

  miFillArcDSetup(arc, &info);
  MIFILLARCSETUP(info, x, y, e, xk, xm, yk, ym, dx, dy, xorg, yorg);
  while (y > 0)
    /* add an upper and maybe a lower span (resp. growing downward, upward) */
    {
      MIFILLARCSTEP(x, y, e, xk, xm, yk, ym, dx, slw); /* y-- */
      ADDSPANS(pts_upper, pts_lower, wids_upper, wids_lower, x, y, slw, e, xk, xorg, yorg, dy);
    }

  numUpperSpans = pts_upper - points_upper;
  numLowerSpans = points_lower + (arc->height - 1) - pts_lower;
  if (numUpperSpans > 0)
    MI_PAINT_SPANS(paintedSet, pGC->pixels[1], numUpperSpans, points_upper, widths_upper)
  if (numLowerSpans > 0)
    MI_COPY_AND_PAINT_SPANS(paintedSet, pGC->pixels[1], numLowerSpans, pts_lower + 1, wids_lower + 1)
  free (points_lower);
  free (widths_lower);
}

#define ADDSPAN(pts, wids, lower, l, r, ya) \
if (r >= l) \
{ \
    pts->x = l; \
    pts->y = ya; \
    *wids = (unsigned int)(r - l + 1); \
    if (lower) \
      { pts--; wids--; } \
    else \
      { pts++; wids++; } \
}

#define ADDSLICESPANS(pts, wids, lower, xl, xr, xorg, ya, flip) \
if (!flip) \
{ \
    ADDSPAN(pts, wids, lower, xl, xr, ya) \
} \
else \
{ \
    int xc; \
    xc = xorg - x; \
    ADDSPAN(pts, wids, lower, xc, xr, ya) \
    xc += (slw - 1); \
    ADDSPAN(pts, wids, lower, xl, xc, ya) \
}

static void
#ifdef _HAVE_PROTOS
miFillArcSliceI (miPaintedSet *paintedSet, const miGC *pGC, const miArc *arc)
#else
miFillArcSliceI (paintedSet, pGC, arc)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     const miArc *arc;
#endif
{
  int yk, xk, ym, xm, dx, dy, xorg, yorg, slw, orig_slw;
  int x, y, e;
  miFillArc info;
  miArcSlice slice;
  int ya, xl, xr;
  miPoint *points_upper, *points_lower, *pts_upper, *pts_lower;
  unsigned int *widths_upper, *widths_lower, *wids_upper, *wids_lower;
  int numUpperSpans, numLowerSpans;

  miFillArcSetup (arc, &info);
  miFillArcSliceSetup (pGC, arc, &slice);
  MIFILLARCSETUP(info, x, y, e, xk, xm, yk, ym, dx, dy, xorg, yorg);
  slw = (int)arc->height;
  if (slice.flip_top || slice.flip_bot)
    slw += (int)(arc->height >> 1) + 1;
  orig_slw = slw;

  points_upper = (miPoint *)mi_xmalloc (sizeof(miPoint) * slw);
  widths_upper = (unsigned int *)mi_xmalloc (sizeof(unsigned int) * slw);
  pts_upper = points_upper;
  wids_upper = widths_upper;
  points_lower = (miPoint *)mi_xmalloc (sizeof(miPoint) * slw);
  widths_lower = (unsigned int *)mi_xmalloc (sizeof(unsigned int) * slw);
  pts_lower = points_lower + (slw - 1);	/* begin at end, will decrement */
  wids_lower = widths_lower + (slw - 1);

  while (y > 0)
    {
      MIFILLARCSTEP(x, y, e, xk, xm, yk, ym, dx, slw); /* y-- */
      MIARCSLICESTEP(slice.edge1);
      MIARCSLICESTEP(slice.edge2);
      if (MIFILLSLICEUPPER(y, slice))
	/* add an `upper' span (growing downward) */
	{
	  bool lower = false;

	  ya = yorg - y;
	  MIARCSLICEUPPER(xl, xr, slice, slw)
	  ADDSLICESPANS(pts_upper, wids_upper, lower, xl, xr, xorg, ya, slice.flip_top)
	}
      if (MIFILLSLICELOWER(y, slice))
	/* add a `lower' span (growing upward) */
	{
	  bool lower = true;

	  ya = yorg + y + dy;
	  MIARCSLICELOWER(xl, xr, slice, slw)
	  ADDSLICESPANS(pts_lower, wids_lower, lower, xl, xr, xorg, ya, slice.flip_bot)
	}
    }

  numUpperSpans = pts_upper - points_upper;
  numLowerSpans = points_lower + (orig_slw - 1) - pts_lower;
  if (numUpperSpans > 0)
    MI_PAINT_SPANS(paintedSet, pGC->pixels[1], numUpperSpans, points_upper, widths_upper)
  if (numLowerSpans > 0)
    MI_COPY_AND_PAINT_SPANS(paintedSet, pGC->pixels[1], numLowerSpans, pts_lower + 1, wids_lower + 1)
  free (points_lower);
  free (widths_lower);
}

static void
#ifdef _HAVE_PROTOS
miFillArcSliceD (miPaintedSet *paintedSet, const miGC *pGC, const miArc *arc)
#else
miFillArcSliceD (paintedSet, pGC, arc)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     const miArc *arc;
#endif
{
  int x, y;
  int dx, dy, xorg, yorg, slw, orig_slw;
  double e, yk, xk, ym, xm;
  miFillArcD info;
  miArcSlice slice;
  int ya, xl, xr;
  miPoint *points_upper, *points_lower, *pts_upper, *pts_lower;
  unsigned int *widths_upper, *widths_lower, *wids_upper, *wids_lower;
  int numUpperSpans, numLowerSpans;

  miFillArcDSetup (arc, &info);
  miFillArcSliceSetup (pGC, arc, &slice);
  MIFILLARCSETUP(info, x, y, e, xk, xm, yk, ym, dx, dy, xorg, yorg);
  slw = (int)arc->height;
  if (slice.flip_top || slice.flip_bot)
    slw += (int)(arc->height >> 1) + 1;
  orig_slw = slw;

  points_upper = (miPoint *)mi_xmalloc (sizeof(miPoint) * slw);
  widths_upper = (unsigned int *)mi_xmalloc (sizeof(unsigned int) * slw);
  pts_upper = points_upper;
  wids_upper = widths_upper;
  points_lower = (miPoint *)mi_xmalloc (sizeof(miPoint) * slw);
  widths_lower = (unsigned int *)mi_xmalloc (sizeof(unsigned int) * slw);
  pts_lower = points_lower + (slw - 1);	/* begin at end, will decrement */
  wids_lower = widths_lower + (slw - 1);

  while (y > 0)
    {
      MIFILLARCSTEP(x, y, e, xk, xm, yk, ym, dx, slw); /* y-- */
      MIARCSLICESTEP(slice.edge1);
      MIARCSLICESTEP(slice.edge2);
      if (MIFILLSLICEUPPER(y, slice))
	/* add an `upper' span (growing downward) */
	{
	  bool lower = false;

	  ya = yorg - y;
	  MIARCSLICEUPPER(xl, xr, slice, slw)
	  ADDSLICESPANS(pts_upper, wids_upper, lower, xl, xr, xorg, ya, slice.flip_top)
	}
      if (MIFILLSLICELOWER(y, slice))
	/* add a `lower' span (growing upward) */
	{
	  bool lower = true;

	  ya = yorg + y + dy;
	  MIARCSLICELOWER(xl, xr, slice, slw)
	  ADDSLICESPANS(pts_lower, wids_lower, lower, xl, xr, xorg, ya, slice.flip_bot)
	}
    }

  numUpperSpans = pts_upper - points_upper;
  numLowerSpans = points_lower + (orig_slw - 1) - pts_lower;
  if (numUpperSpans > 0)
    MI_PAINT_SPANS(paintedSet, pGC->pixels[1], numUpperSpans, points_upper, widths_upper)
  if (numLowerSpans > 0)
    MI_COPY_AND_PAINT_SPANS(paintedSet, pGC->pixels[1], numLowerSpans, pts_lower + 1, wids_lower + 1)
  free (points_lower);
  free (widths_lower);
}

void
#ifdef _HAVE_PROTOS
miFillArcs_internal (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs)
#else
miFillArcs_internal (paintedSet, pGC, narcs, parcs)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int narcs;
     const miArc *parcs;
#endif
{
  int i;
  const miArc *arc;

  for (i = narcs, arc = parcs; --i >= 0; arc++)
    {
      if (MI_FILLED_ARC_IS_EMPTY(arc))
	continue;

      if ((arc->angle2 >= FULLCIRCLE) || (arc->angle2 <= -FULLCIRCLE))
	{
	  if (MI_CAN_FILL_ARC(arc))
	    /* circle; or width, height <= 800 */
	    miFillEllipseI (paintedSet, pGC, arc);
	  else
	    miFillEllipseD (paintedSet, pGC, arc);
	}
      else
	{
	  if (MI_CAN_FILL_ARC(arc))
	    /* circle; or width, height <= 800 */
	    miFillArcSliceI (paintedSet, pGC, arc);
	  else
	    miFillArcSliceD (paintedSet, pGC, arc);
	}
    }
}
