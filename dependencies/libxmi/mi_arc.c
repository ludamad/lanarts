#include "sys-defines.h"
#include "extern.h"

/* Authors: Keith Packard and Bob Scheifler, mid to late 1980s.
   Hacked by Robert S. Maier <rsm@math.arizona.edu>, 1998-2000. */

/* This module exports the miPolyArc() function and its reentrant
   counterpart miPolyArc_r.  They scan-convert wide polyarcs, either solid
   or dashed.  A polyarc is a list of arcs, which may or may not be
   contiguous.  Here, an `arc' is an elliptic arc, i.e., a segment of an
   ellipse.  The principal axes of the ellipse must be aligned with the
   coordinate axes.
   
   Each arc is drawn with a circular brush, of width equal to the line
   width.  All pixels within the brushed area are painted.

   All painting goes through the low-level fillSpans() function and the
   MI_PAINT_SPANS() macro that it invokes, except that the miFillSppPoly()
   routine in mi_fplycon.c is used to paint polygonal arc caps and arc
   joins, if any.  That routine, in turn, invokes MI_PAINT_SPANS(). */

/* Warning: this code is toxic, do not dally very long here. */

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"
#include "mi_arc.h"
#include "mi_fply.h"		/* for miFillSppPoly() */
#include "mi_fllarc.h"		/* for MIWIDEARCSETUP, MIFILLINARCSTEP etc. */

/* undefine if cbrt(), a fast cube root function for non-negative
   arguments, is available */
#define cbrt(x) pow((x), 1.0/3.0)

/* undefine if hypot is available (it's X_OPEN, but not ANSI or POSIX) */
#define hypot(x, y) sqrt((x)*(x) + (y)*(y))

/**********************************************************************/

/* To facilitate speedy drawing of elliptic arcs, we cache scan-converted
   wide ellipses so that we can retrieve them later, by keying on ellipse
   width, ellipse height, and line width.  Any such cache is an
   miEllipseCache object; equivalently, a lib_miEllipseCache structure,
   which is basically an array of (cachedEllipse *)'s.  Each cachedEllipse
   is a record, the `value' field of which is an (miArcSpanData *),
   i.e. basically a list of spans, computed and returned by
   miComputeWideEllipse().

   The currently used miEllipseCache structure is accessed via the
   ellipseCache argument of miPolyArc_r(). */

/* one or two spans (any rasterized ellipse contains a list of these,
   indexed by y) */
typedef struct 
{
  int lx, rx;			/* starting points of left, right spans */
  int lw, rw;			/* widths of left, right spans */
} miArcSpan;

/* `Value' part of each cache record, storing a rasterized ellipse.  A
   rasterized ellipse is a list of ArcSpans, extending in order from the
   top of the ellipse down to and including the centerline, if present.  It
   consists primarily of count1 single-occupied ArcSpans (containing only a
   single span), followed by count2 doubly-occupied ArcSpans (containing
   two spans).

   In all, the list contains (top ? 1 : 0) + count1 + count2 + (bot ? 1 : 0)
   ArcSpans.  The x-coordinates (lx,rx) in the ArcSpans are relative
   to x = xorg = xorg_arc + width_arc/2, i.e. halfway across the ellipse.  
   The count1 single-occupied ArcSpans are drawn downward (i.e. at 
   successively increasing values of y), beginning at
   y = yorgu = yorg_arc + height_arc/2 - k, and also upward beginning at
   y = yorgl = yorg_arc + height_arc/2 + k.
   They are followed by the count2 doubly occupied ArcSpans.

   (Here k = height_arc/2 + (linewidth-1)/2, always.)

   If top=true (which is the case iff width_arc is even and linewidth is
   even), the first ArcSpan in the array is bogus, and should be replaced
   by a single pixel at x=xorg, y = yorgu-1.  If bot=true, which is the
   case iff height_arc is even, then the ellipse must be completed by
   drawing the 2 or 1 spans contained in the final ArcSpan of the array on
   the vertical centerline.  (rw<0 is a signal that this ArcSpan atypically
   contains only a single span.)

   `hole' is a kludge flag.  If it is set, then after the count1 singly
   occupied ArcSpans are drawn upward, a single additional pixel must be
   drawn at (xorg,y), where y is the current (updated, i.e. decremented)
   value of y.  y is not further decremented before the drawing of the
   count2 doubly occupied ArcSpans begins. */

typedef struct 
{
  int k;			/* height/2 + (linewidth-1)/2, always */
  miArcSpan *spans;		/* array of up to k+2 miArcSpan structures */
  bool top;			/* have initial (bogus) ArcSpan? */
  int count1;			/* number of single-occupancy ArcSpans */
  int count2;			/* number of double-occupancy ArcSpans */
  bool bot;			/* have final (special) ArcSpan? */
  bool hole;			/* add a certain pixel when drawing? */
} miArcSpanData;

/* Cache record type (key/value); key consists of width,height,linewidth.
   Also includes a timestamp. */
typedef struct 
{
  unsigned long lrustamp;	/* timestamp (time of most recent retrieval) */
  unsigned int width, height;	/* ellipse width, height */
  unsigned int lw;		/* line width used when rasterizing */
  miArcSpanData *spdata;	/* `value' part of record */
} cachedEllipse;

/* The cache of scan-converted ellipses, including continually updated
   timestamp. */
struct lib_miEllipseCache
{
  cachedEllipse *ellipses;	/* beginning of array of records */
  int size;			/* number of records in array */
  cachedEllipse *lastCacheHit;	/* pointer to last-accessed record */
  unsigned long lrustamp;	/* clock, for timestamping records */
};

/* Size of cache (i.e. number of (cachedEllipse *)'s the array contains) */
#define ELLIPSECACHE_SIZE 25

/* Maximum height an ellipse can have, for its spans to be stored in
   the cache. */
#define MAX_CACHEABLE_ELLIPSE_HEIGHT 1500

#ifndef NO_NONREENTRANT_POLYARC_SUPPORT
/* An in-library cache, used by the non-reentrant functions miPolyArc()
   and miZeroPolyArc(). */
miEllipseCache *_mi_ellipseCache = (miEllipseCache *)NULL;
#endif /* NO_NONREENTRANT_POLYARC_SUPPORT */

/**********************************************************************/

/* We must scan-convert poly-arcs, which consist of one or more wide
   elliptic arcs, which may or may not be contiguous, and which may or may
   not be dashed.  In this, the function miComputeArcs() plays a key role.
   It doesn't do scan conversion.  Instead, it chops an ellipse into arcs
   or small arcs representing dashes, determines whether joins or caps are
   called for, etc.  What it returns is a list of miPolyArcs structures,
   indexed by paint type.  

   A miPolyArcs structure comprises a list of miArcData structs, a list of
   joins, and a list of caps. */

/* Note that self intersecting arcs (i.e. those spanning 360 degrees) never
   join with other arcs, and are drawn without caps (unless on/off dashed,
   in which case each dash segment is capped, except when the last segment
   meets the first segment, when no caps are drawn). */

#define RIGHT_END	0
#define LEFT_END	1

typedef struct
{
  int	arcIndex0, arcIndex1;	/* arcs to either side of the join */
  int	paintType0, paintType1;	/* their paint types */
  int	end0, end1;		/* either RIGHT_END or LEFT_END */
} miArcJoinStruct;

typedef struct
{
  int	arcIndex;		/* arc to be capped */
  int	end;			/* either RIGHT_END or LEFT_END */
} miArcCapStruct;

typedef struct
{
  SppPoint	clock;
  SppPoint	center;
  SppPoint	counterClock;
} miArcFace;

typedef struct
{
  miArc		arc;
  bool		render;		/* directive to render this arc (and
				   all previously non-rendered ones) */
  int		join;		/* related join */
  int		cap;		/* related cap */
  bool		selfJoin;	/* arc is self-joining? */
  miArcFace	bounds[2];	/* left face and right face (3 points each) */
  double	x0, y0;		/* starting point (sub-pixel placement) */
  double	x1, y1;		/* end point (sub-pixel placement) */
} miArcData;

/*
 * The miPolyArcs struct.  This is a sequence of arcs (e.g., dashes),
 * computed and categorized according to operation.  miComputeArcs()
 * returns a list of these, indexed by paint type.  */

typedef struct
{
  miArcData		*arcs;
  int			narcs;
  int			arcSize; /* number of slots allocated */
  miArcCapStruct	*caps;
  int			ncaps;
  int			capSize; /* number of slots allocated */
  miArcJoinStruct	*joins;
  int			njoins;
  int			joinSize; /* number of slots allocated */
} miPolyArcs;

/**********************************************************************/

/* In a sub-module below, with public functions initAccumSpans(),
   newFinalSpan(), and fillSpans(), we initialize an miAccumSpans
   structure, add spans to it, and finally paint and deallocate them.

   The miAccumSpans struct includes an array, indexed by y-value, each
   element of which is a list of spans.  The y range, i.e. the range of the
   index variable of the array, is expanded as needed.

   To facilitate rapid addition of spans to the structure, we maintain as
   part of the miAccumSpans structure a list of unused span structures,
   previously allocated in "chunks".  */

struct finalSpan 
{
  int			min, max; /* x values */
  struct finalSpan	*next;	/* pointer to next span at this value of y */
};

#define SPAN_CHUNK_SIZE 128	/* spans are malloc'd in chunks of this size */
struct finalSpanChunk 
{
  struct finalSpan	data[SPAN_CHUNK_SIZE];
  struct finalSpanChunk	*next;	/* pointer to previously malloc'd chunk */
};

typedef struct
{
  struct finalSpan **finalSpans;         /* array, indexed by y - finalMiny */
  int              finalMiny, finalMaxy; /* y range */
  int              finalSize;
  int              nspans;	 /* total number of spans, not just y coors */
  struct finalSpanChunk	*chunks; /* head of chunk list */
  struct finalSpan *freeFinalSpans; /* next free span in chunk at list head */
} miAccumSpans;

/**********************************************************************/

/* Structure used by a sub-module that computes arc lengths via a polygonal
   approximation to the arc.  The sub-module's external functions are
   computeDashMap() and computeAngleFromPath(). */

#define DASH_MAP_SIZE 91
typedef struct 
{
  double	map[DASH_MAP_SIZE];
} dashMap;

/**********************************************************************/

/* internal functions that do painting of pixels */
static void fillSpans ____P((miPaintedSet *paintedSet, miPixel pixel, miAccumSpans *accumSpans));
static void miArcCap ____P((miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, const miArcFace *pFace, int end, int xOrg, int yOrg, double xFtrans, double yFtrans));
static void miArcJoin ____P((miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, const miArcFace *pLeft, const miArcFace *pRight, int xOrgLeft, int yOrgLeft, double xFtransLeft, double yFtransLeft, int xOrgRight, int yOrgRight, double xFtransRight, double yFtransRight));
static void miFillWideEllipse ____P((miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, const miArc *parc, miEllipseCache *ellipseCache));
static void miRoundCap ____P((miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, SppPoint pCenter, SppPoint pEnd, SppPoint pCorner, SppPoint pOtherCorner, int fLineEnd, int xOrg, int yOrg, double xFtrans, double yFtrans));

/* internal functions that don't do painting of pixels */
static double angleBetween ____P((SppPoint center, SppPoint point1, SppPoint point2));
static double miDasin ____P((double v));
static double miDatan2 ____P((double dy, double dx));
static double miDcos ____P((double a));
static double miDsin ____P((double a));
static int computeAngleFromPath ____P((int startAngle, int endAngle, const dashMap *map, int *lenp, bool backwards));
static int miGetArcPts ____P((const SppArc *parc, int cpt, SppPoint **ppPts));
static miArcData * addArc ____P((miPolyArcs *polyArcs, const miArc *xarc));
static miArcSpanData * miComputeWideEllipse ____P((unsigned int lw, const miArc *parc, bool *mustFree, miEllipseCache *ellipseCache));
static miPolyArcs * miComputeArcs ____P((const miGC *pGC, const miArc *parcs, int narcs));
static void addCap ____P((miPolyArcs *polyArcs, int end, int arcIndex));
static void addJoin ____P((miPolyArcs *polyArcs, int end0, int index0, int paintType0, int end1, int index1, int paintType1));
static void computeDashMap ____P((const miArc *arcp, dashMap *map));
static void drawArc ____P((miAccumSpans *accumSpans, const miArc *tarc, unsigned int l, int a0, int a1, miArcFace *right, miArcFace *left, miEllipseCache *ellipseCache));
static void drawZeroArc ____P((miAccumSpans *accumSpans, const miArc *tarc, unsigned int lw, miArcFace *left, miArcFace *right));
static void initAccumSpans ____P((miAccumSpans *accumSpans));
static void miArcSegment ____P((const miGC *pGC, miAccumSpans *accumSpans, miArc tarc, miArcFace *right, miArcFace *left, miEllipseCache *ellipseCache));
static void miComputeCircleSpans ____P((unsigned int lw, const miArc *parc, miArcSpanData *spdata));
static void miComputeEllipseSpans ____P((unsigned int lw, const miArc *parc, miArcSpanData *spdata));
static void miFreeArcs ____P((const miGC *pGC, miPolyArcs *arcs));
static void translateBounds ____P((miArcFace *b, int x, int y, double fx, double fy));


/*
 * Comments on overall miPolyArc/miPolyArc_r strategy:
 *
 * If the arc is zero width and solid, we don't worry about the join style.
 * To scan-convert wide solid circles, we use a fast integer algorithm.  To
 * scan-convert wide solid ellipses, we use special case floating point
 * code.
 *
 * The scan-conversion of wide circles and ellipse is comparatively
 * trivial, though the latter involves some polynomial algebra.  The
 * greater part of the code deals with chopping circles and ellipses,
 * rasterized or not, into segments.  This includes dashing.
 *
 * This function is the reentrant version, miPolyArc_r.  The non-reentrant
 * version, miPolyArc, maintains its own `rasterized ellipse' cache as
 * static data, and simply calls this one.  */

void
#ifdef _HAVE_PROTOS
miPolyArc_r (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs, miEllipseCache *ellipseCache)
#else
miPolyArc_r (paintedSet, pGC, narcs, parcs, ellipseCache)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int narcs;
     const miArc *parcs;
     miEllipseCache *ellipseCache; /* pointer to ellipse data cache */
#endif
{
  int           i;
  const miArc   *parc;
  int           width;
  miPixel	pixel;
  miPolyArcs    *polyArcs;
  int           *cap, *join;
  int           paintType;
  miAccumSpans	accumSpans_struct; /* in-core accumulation of spans */

  /* ensure we have >=1 arcs */
  if (narcs <= 0)
    return;

  /* Initialize miAccumSpans structure (painted to by the low-level drawArc
     function).  N.B. After drawArc() is repeatedly called to fill up the
     miAccumSpans struct with spans of a single paint type, fillSpans() is
     called with the desired paint type as one of its arguments.  It will
     `paint' from the miAccumSpans struct to the miPaintedSet. */
  initAccumSpans (&accumSpans_struct);
  
  pixel = pGC->pixels[1];	/* default single pixel color to use */

  width = pGC->lineWidth;
  if (width == 0 && pGC->lineStyle == (int)MI_LINE_SOLID)
    /* zero-width solid arcs, only */
    {
      for (i = narcs, parc = parcs; --i >= 0; parc++)
	/* Draw zero-width arc segment to the miAccumSpans struct, as a set
	   of spans, by invoking the low-level drawArc() function.  This
	   updates the ellipse span data cache. */
	miArcSegment (pGC,
		      &accumSpans_struct, *parc, 
		      (miArcFace *)NULL, (miArcFace *)NULL,
		      ellipseCache);
      /* `paint' the arc segments in the miAccumSpans struct (i.e. add them
	 to the miPaintedSet struct), in the current pixel color */
      fillSpans (paintedSet, pixel, &accumSpans_struct);
    }
  else				/* nonzero width, or dashing */
    {
      if ((pGC->lineStyle == (int)MI_LINE_SOLID) && narcs)
	{
	  /* first, paint any initial complete ellipses (for speed) */
	  while (parcs->width && parcs->height
		 && (parcs->angle2 >= FULLCIRCLE ||
		     parcs->angle2 <= -FULLCIRCLE))
	    {
	      /* paint complete ellipse without going through the
                 miAccumSpans struct, also update ellipse span data cache */
	      miFillWideEllipse (paintedSet, pixel, pGC,
				 parcs, ellipseCache);
	      if (--narcs == 0)
		return;
	      parcs++;
	    }
	}

      /* have one or more elliptic arcs that are incomplete ellipses
         (possibly dashed, possibly contiguous) to draw */

      /* compute arc segments (i.e. dashes) in the incomplete ellipses,
	 indexed by color; will need to be freed with miFreeArcs() */
      polyArcs = miComputeArcs (pGC, parcs, narcs);

      cap = (int *) mi_xmalloc (pGC->numPixels * sizeof(int));
      join = (int *) mi_xmalloc (pGC->numPixels * sizeof(int));
      for (i = 0; i < pGC->numPixels; i++)
	cap[i] = join[i] = 0;

      /* iterate over colors, drawing arc segments in each color */
      for (paintType = 0; paintType < pGC->numPixels; paintType++)
	{
	  pixel = pGC->pixels[paintType];
	  for (i = 0; i < polyArcs[paintType].narcs; i++) 
	    {
	      miArcData *arcData;
	      
	      /* Draw an arc segment to the miAccumSpans struct, via
		 drawArc() */
	      arcData = &polyArcs[paintType].arcs[i];
	      miArcSegment (pGC,
			    &accumSpans_struct, arcData->arc,
			    &arcData->bounds[RIGHT_END],
			    &arcData->bounds[LEFT_END],
			    ellipseCache);
	      if (polyArcs[paintType].arcs[i].render) 
		{
		  /* `paint' the arc, and any arcs previously drawn to
		     the miAccumSpans struct but not painted, to the
		     miPaintedSet struct, in the current pixel color */
		  fillSpans (paintedSet, pixel, &accumSpans_struct);

		  /* `paint' all undrawn caps to the miPaintedSet struct */
		  
		  /* test for self-joining arcs (won't be capped) */
		  if (polyArcs[paintType].arcs[i].selfJoin
		      && cap[paintType] < polyArcs[paintType].arcs[i].cap)
		    cap[paintType]++;
		  while (cap[paintType] < polyArcs[paintType].arcs[i].cap) 
		    {
		      int	arcIndex, end;
		      miArcData *arcData0;
									     
		      arcIndex = polyArcs[paintType].caps[cap[paintType]].arcIndex;
		      end = polyArcs[paintType].caps[cap[paintType]].end;
		      arcData0 = &polyArcs[paintType].arcs[arcIndex];
		      /* `paint' cap to the miPaintedSet struct by invoking
                         miFillSppPoly() */
		      miArcCap (paintedSet, pixel, pGC,
				&arcData0->bounds[end], end,
				arcData0->arc.x, arcData0->arc.y,
				(double)(0.5 * arcData0->arc.width),
				(double)(0.5 * arcData0->arc.height));
		      ++cap[paintType];
		    }

		  /* `paint' all undrawn joins to the miPaintedSet struct */

		  while (join[paintType] < polyArcs[paintType].arcs[i].join) 
		    {
		      int	arcIndex0, arcIndex1, end0, end1;
		      int	paintType0, paintType1;
		      miArcData *arcData0, *arcData1;
		      miArcJoinStruct *joinp;

		      joinp = &polyArcs[paintType].joins[join[paintType]];
		      arcIndex0 = joinp->arcIndex0;
		      end0 = joinp->end0;
		      arcIndex1 = joinp->arcIndex1;
		      end1 = joinp->end1;
		      paintType0 = joinp->paintType0;
		      paintType1 = joinp->paintType1;
		      arcData0 = &polyArcs[paintType0].arcs[arcIndex0];
		      arcData1 = &polyArcs[paintType1].arcs[arcIndex1];
		      /* `paint' join to the miPaintedSet struct by invoking
                         miFillSppPoly() */
		      miArcJoin (paintedSet, pixel, pGC,
				 &arcData0->bounds[end0],
				 &arcData1->bounds[end1],
				 arcData0->arc.x, arcData0->arc.y,
				 (double) (0.5 * arcData0->arc.width),
				 (double) (0.5 * arcData0->arc.height),
				 arcData1->arc.x, arcData1->arc.y,
				 (double) (0.5 * arcData1->arc.width),
				 (double) (0.5 * arcData1->arc.height));
		      ++join[paintType];
		    }
		}
	    }

	}
      free (cap);
      free (join);

      /* free arc segments computed by miComputeArcs() */
      miFreeArcs (pGC, polyArcs);
    }
}

#ifndef NO_NONREENTRANT_POLYARC_SUPPORT
/* The non-reentrant version of miPolyArc, which unlike miPolyArc_r
   maintains its own ellipse spans cache as static (persistent) data. */
void
#ifdef _HAVE_PROTOS
miPolyArc (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs)
#else
miPolyArc (paintedSet, pGC, narcs, parcs)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int narcs;
     const miArc *parcs;
#endif
{
  if (_mi_ellipseCache == (miEllipseCache *)NULL)
    _mi_ellipseCache = miNewEllipseCache ();
  miPolyArc_r (paintedSet, pGC, narcs, parcs, _mi_ellipseCache);
}
#endif /* not NO_NONREENTRANT_POLYARC_SUPPORT */

/* Initialize a cache of rasterized elliptic arcs.  (A pointer to such an
   object is passed to miPolyArc_r.)  Such a cache comprises an array of
   records (i.e. cachedEllipse's), a pointer to one of them (the most
   recently used), and a timestamp variable that is incremented when any
   record is cached.  `Replace least recently used' is the policy. */
miEllipseCache *
#ifdef _HAVE_PROTOS
miNewEllipseCache (void)
#else
miNewEllipseCache ()
#endif
{
  int k;
  cachedEllipse *chead, *cent;
  miEllipseCache *ellipseCache;

  ellipseCache = (miEllipseCache *)mi_xmalloc (sizeof(miEllipseCache));

  /* pointer to beginning of array of records */
  ellipseCache->ellipses = (cachedEllipse *)mi_xmalloc (ELLIPSECACHE_SIZE * sizeof(cachedEllipse));
  /* length of array */
  ellipseCache->size = ELLIPSECACHE_SIZE;
  /* pointer to beginning of last-accessed record (a dummy value) */
  ellipseCache->lastCacheHit = ellipseCache->ellipses;
  /* clock for timestamping */
  ellipseCache->lrustamp = 0;

  /* initialize elements of each record with null/bogus values */
  chead = ellipseCache->ellipses;
  for (k = ELLIPSECACHE_SIZE, cent = chead; --k >= 0; cent++)
    {
      cent->lrustamp = 0;
      cent->lw = 0;
      cent->width = cent->height = 0;
      cent->spdata = (miArcSpanData *)NULL;
    }

  return ellipseCache;
}

/* Free a cache of rasterized ellipses, which must previously have been
   allocated by invoking miNewEllipseCache. */
void
#ifdef _HAVE_PROTOS
miDeleteEllipseCache (miEllipseCache *ellipseCache)
#else
miDeleteEllipseCache (ellipseCache)
     miEllipseCache *ellipseCache;
#endif
{
  int k, cache_size;
  cachedEllipse *chead, *cent;

  /* free span data in all records */
  chead = ellipseCache->ellipses;
  cache_size = ellipseCache->size;
  for (k = cache_size, cent = chead; --k >= 0; cent++)
    {
      miArcSpanData *spdata;
      
      spdata = cent->spdata;
      if (spdata)
	{
	  free (spdata->spans);
	  free (spdata);
	}
    }
  /* free the record array itself */
  free (chead);

  /* free pointer */
  free (ellipseCache);
}

/* Draw a single arc segment to an miAccumSpans struct, via drawArc() or
 * drawZeroArc().  Right and left faces may be specified, for mirroring
 * purposes (they're usually computed by miComputeArcs()).  The
 * accumulation of spans in the miAccumSpans struct will need to be painted
 * by a later invocation of fillSpans().  This function updates the ellipse
 * span cache. */
static void
#ifdef _HAVE_PROTOS
miArcSegment (const miGC *pGC, miAccumSpans *accumSpans, miArc tarc, miArcFace *right, miArcFace *left, miEllipseCache *ellipseCache)
#else
miArcSegment (pGC, accumSpans, tarc, right, left, ellipseCache)
     const miGC *pGC;
     miAccumSpans *accumSpans;
     miArc tarc;
     miArcFace *right, *left;
     miEllipseCache *ellipseCache;
#endif
{
  unsigned int l = pGC->lineWidth;
  int a0, a1, startAngle, endAngle;
  miArcFace *temp;

  if (l == 0)			/* map zero width to unit width */
    l = 1;
  
  if (tarc.width == 0 || tarc.height == 0) 
    {
      /* degenerate case, either horizontal or vertical arc */
      drawZeroArc (accumSpans, &tarc, l, left, right);
      return;
    }
  
  a0 = tarc.angle1;
  a1 = tarc.angle2;
  if (a1 > FULLCIRCLE)
    a1 = FULLCIRCLE;
  else if (a1 < -FULLCIRCLE)
    a1 = -FULLCIRCLE;
  if (a1 < 0) 
    {
      startAngle = a0 + a1;
      endAngle = a0;
      temp = right;
      right = left;
      left = temp;
    } 
  else 
    {
      startAngle = a0;
      endAngle = a0 + a1;
    }
  /*
   * bounds check the two angles
   */
  if (startAngle < 0)
    startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
  if (startAngle >= FULLCIRCLE)
    startAngle = startAngle % FULLCIRCLE;
  if (endAngle < 0)
    endAngle = FULLCIRCLE - (-endAngle) % FULLCIRCLE;
  if (endAngle > FULLCIRCLE)
    endAngle = (endAngle-1) % FULLCIRCLE + 1;
  if ((startAngle == endAngle) && a1) 
    {
      startAngle = 0;
      endAngle = FULLCIRCLE;
    }
  
  /* Draw the arc to memory, as a set of spans (accumulated spans must
     later be painted and deallocated by invoking fillSpans()).  This
     updates the ellipse span cache. */
  drawArc (accumSpans,
	   &tarc, l, startAngle, endAngle, right, left, 
	   ellipseCache);
}

/* Paint a wide, complete [i.e. undashed] ellipse, immediately.  I.e.,
   paint it to a miPaintedSet, not to an in-core miAccumSpans struct.
   Called by miPolyArc if angle is at least 360 degrees.  Calls
   miComputeWideEllipse(), and updates the ellipse span cache. */
static void
#ifdef _HAVE_PROTOS
miFillWideEllipse (miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, const miArc *parc, miEllipseCache *ellipseCache)
#else
miFillWideEllipse (paintedSet, pixel, pGC, parc, ellipseCache)
     miPaintedSet *paintedSet;
     miPixel pixel;
     const miGC *pGC;
     const miArc *parc;
     miEllipseCache *ellipseCache;
#endif
{
  miArcSpanData *spdata;
  bool mustFree;
  miArcSpan *arcSpan, *finalArcSpan;
  int xorg, yorgu, yorgl;
  int numArcSpans, n;
  int numSpans_downward, numSpans_upward, numSpans, botSpans;
  miPoint *pptInit, *ppt_downward, *ppt_upward;
  unsigned int *pwidthInit, *pwidth_downward, *pwidth_upward;

  /* Compute span data for whole wide ellipse, updating the ellipse cache.
     Return value will be a pointer to a miArcSpanData struct, which is
     basically an array of miArcSpan's indexed by y.  A miArcSpan comprises
     one or two spans. */
  spdata = miComputeWideEllipse (pGC->lineWidth, parc, &mustFree, ellipseCache);
  if (!spdata)
    /* unknown failure, so punt */
    return;

  arcSpan = spdata->spans;	/* first ArcSpan in array */

  /* initialize upper and lower y values for span generation;
     note spdata->k = height/2 + (linewidth-1)/2, always */
  xorg = parc->x + (int)(parc->width >> 1);
  yorgu = parc->y + (int)(parc->height >> 1);
  yorgl = yorgu + (parc->height & 1);
  yorgu -= spdata->k;
  yorgl += spdata->k;

  /* Add spans both from top of the ellipse (growing downward from y=yorgu)
     and from bottom (growing upward from y=yorgl), computed from the
     (top ? 1 : 0) + count1 + count2 + (bottom ? 1 : 0) 
     ArcSpans contained in spdata->spans.

     Number of `downward' spans = (top ? 1 : 0) + count1 + 2*count2
                                  + (bot ? [1or2] : 0)
     Number of `upward' spans = count1 + (hole ? 1 : 0) + 2*count2

     Here [1or2] = (finalArcSpan->rw <= 0 ? 1 : 2), where `finalArcSpan' is
     the final ArcSpan in the array spdata->spans.  These final 1 or 2
     spans, if present, are on the horizontal centerline of the ellipse.

     N.B. Presumably 
     (top ? 1 : 0) + count1 + count2 + (bottom ? 1 : 0) <= k+2,
     since the ArcSpans array spdata->spans can contain at most k+2 ArcSpans,
     as allocated (see miComputeWideEllipse()). */

  numArcSpans = ((spdata->top ? 1 : 0) + spdata->count1 
		 + spdata->count2 + (spdata->bot ? 1 : 0));
  finalArcSpan = &(spdata->spans[numArcSpans - 1]);
  botSpans = (finalArcSpan->rw <= 0 ? 1 : 2);

  numSpans_downward = ((spdata->top ? 1 : 0) 
		       + spdata->count1 + 2 * spdata->count2 
		       + (spdata->bot ? botSpans : 0));
  numSpans_upward = (spdata->count1 + (spdata->hole ? 1 : 0) 
		     + 2 * spdata->count2);
  numSpans = numSpans_downward + numSpans_upward;

  /* allocate span array; will fill it from both ends, so that it will be
     sorted (i.e. in y-increasing order) */
  pptInit = (miPoint *)mi_xmalloc (numSpans * sizeof(miPoint));
  pwidthInit = (unsigned int *)mi_xmalloc (numSpans * sizeof(unsigned int));
  ppt_downward = pptInit;
  pwidth_downward = pwidthInit;
  ppt_upward = pptInit + (numSpans - 1);
  pwidth_upward = pwidthInit + (numSpans - 1);

  if (spdata->top)		/* true if width is even and lw is even */
    /* begin with a special `top point' at y=yorgu-1, rather than at
       y=yorgu; skip first ArcSpan (it may be bogus) */
    {
      ppt_downward->x = xorg;
      ppt_downward->y = yorgu - 1;
      ppt_downward++;
      *pwidth_downward++ = 1;
      arcSpan++;
    }

  for (n = spdata->count1; --n >= 0; )
    /* Add successive pairs of spans, one upper [beginning at y=yorgu], one
       lower [beginning at y=yorgl].  Each pair is taken from one of the
       next count1 ArcSpans in spdata; these ArcSpans are singly occupied.  */
    {
      ppt_downward->x = xorg + arcSpan->lx;
      ppt_downward->y = yorgu;
      *pwidth_downward = arcSpan->lw;
      ppt_downward++;
      pwidth_downward++;

      ppt_upward->x = xorg + arcSpan->lx;
      ppt_upward->y = yorgl;
      *pwidth_upward = arcSpan->lw;
      ppt_upward--;
      pwidth_upward--;

      yorgu++;
      yorgl--;
      arcSpan++;
    }

  if (spdata->hole)
    /* Kludge: add a single additional lower point, at x=xorg, y=yorgl (now
       decremented), i.e. on the vertical center line.  Do not decrement
       yorgl further, i.e. do not move upward.  (So this extra point will
       fit between the two spans of the next `upward' ArcSpan to be drawn.)  */
    {
      ppt_upward->x = xorg;
      ppt_upward->y = yorgl;
      *pwidth_upward = 1;
      ppt_upward--;
      pwidth_upward--;
    }

  for (n = spdata->count2; --n >= 0; )
    /* add four spans, two above, two below (each quad taken from one of
       the next count2 ArcSpans in spdata; these ArcSpans are doubly
       occupied, containing both a left and a right span) */
    {
      /* left downward span */
      ppt_downward->x = xorg + arcSpan->lx;
      ppt_downward->y = yorgu;
      *pwidth_downward = arcSpan->lw;
      ppt_downward++;
      pwidth_downward++;
      /* right downward span */
      ppt_downward->x = xorg + arcSpan->rx;
      ppt_downward->y = yorgu;
      *pwidth_downward = arcSpan->rw;
      ppt_downward++;
      pwidth_downward++;
      /* left upward span */
      ppt_upward->x = xorg + arcSpan->lx;
      ppt_upward->y = yorgl;
      *pwidth_upward = arcSpan->lw;
      ppt_upward--;
      pwidth_upward--;
      /* right upward span */
      ppt_upward->x = xorg + arcSpan->rx;
      ppt_upward->y = yorgl;
      *pwidth_upward = arcSpan->rw;
      ppt_upward--;
      pwidth_upward--;

      yorgu++;
      yorgl--;
      arcSpan++;
    }

  if (spdata->bot)		/* true if height is even */
    /* To complete the ellipse, add 1 or 2 additional `upper' spans, at
       y=yorgu (incremented, i.e. it is now at the horizontal center line,
       which is at y = yorg_arc + height_arc/2).  The number of spans will
       be 2 rather than 1, unless the ellipse is not hollow. */
    {
      ppt_downward->x = xorg + arcSpan->lx;
      ppt_downward->y = yorgu;
      *pwidth_downward = arcSpan->lw;
      ppt_downward++;
      pwidth_downward++;

      if (arcSpan->rw > 0)
	/* have a second span too */
	{
	  ppt_downward->x = xorg + arcSpan->rx;
	  ppt_downward->y = yorgu;
	  *pwidth_downward = arcSpan->rw;
	  ppt_downward++;
	  pwidth_downward++;
	}
    }

  if (mustFree)			/* free the ArcSpans */
    {
      free (spdata->spans);
      free (spdata);
    }

  MI_PAINT_SPANS(paintedSet, pixel, numSpans, pptInit, pwidthInit)
}

/* Compute the spans that make up a wide complete ellipse, this way: (1)
   search the cache of rasterized ellipses; if no success, (2) scan-convert
   the ellipse, and place the spans in the cache for later retrieval, in
   case another ellipse of the same size and width needs to be drawn.

   Return value will be a pointer to a miArcSpanData struct, which is
   basically an array of miArcSpan's indexed by y.  A miArcSpan comprises
   lx, lwidth, rx, rwidth, i.e., a pair of spans at a given y.  `mustFree'
   will be set to true if the miArcSpanData struct is a one-shot creation,
   not stored in the cache.  (This is the case if the ellipse is too large
   to be stored in the cache -- a policy issue.)

   Called by the low-level draw-to-memory function drawArc(), and also by
   miFillWideEllipse(), which paints an entire wide ellipse.  This function
   calls either miComputeEllipseSpans() or miComputeCircleSpans() to do the
   actual computation of spans, i.e. to do scan conversion.  */

static miArcSpanData *
#ifdef _HAVE_PROTOS
miComputeWideEllipse (unsigned int lw, const miArc *parc, bool *mustFree, miEllipseCache *ellipseCache)
#else
miComputeWideEllipse (lw, parc, mustFree, ellipseCache)
     unsigned int lw;
     const miArc *parc;
     bool *mustFree;
     miEllipseCache *ellipseCache;
#endif
{
  miArcSpanData *spdata;
  cachedEllipse *cent, *lruent;
  int k, cache_size;
  cachedEllipse fakeent;

  /* map zero line width to width unity */
  if (lw == 0)
    lw = 1;

  /* first, attempt to retrieve span data from cache */
  if (parc->height <= MAX_CACHEABLE_ELLIPSE_HEIGHT)
    {
      *mustFree = false;
      cent = ellipseCache->lastCacheHit;
      if (cent->lw == lw 
	  && cent->width == parc->width && cent->height == parc->height)
	/* last hit is still valid; won't need to search */
	{
	  /* hit again; do timestamp, bumping time */
	  cent->lrustamp = ++(ellipseCache->lrustamp);
	  return cent->spdata;
	}
      /* search cache (an array), beginning at 0'th element */
      lruent = ellipseCache->ellipses;
      cache_size = ellipseCache->size;
      for (k = cache_size, cent = lruent; --k >= 0; cent++)
	{
	  /* key on width, height, linewidth */
	  if (cent->lw == lw 
	      && cent->width == parc->width && cent->height == parc->height)
	    /* already in cache: a hit */
	    {
	      /* do timestamp, bumping time */
	      cent->lrustamp = ++(ellipseCache->lrustamp);
	      ellipseCache->lastCacheHit = cent;
	      return cent->spdata;
	    }
	  /* keep track of least recently used record */
	  if (cent->lrustamp < lruent->lrustamp)
	    lruent = cent;
	}
    } 
  else /* height is huge, ellipse wouldn't be stored in cache */
    {
      lruent = &fakeent;	/* _very_ fake; automatic variable */
      lruent->spdata = (miArcSpanData *)NULL;
      *mustFree = true;
    }

  /* data not found in cache, so boot least-recently used record out of
     cache, make new one; unless ellipse is too large, that is */

  spdata = lruent->spdata;
  /* will allocate space for k+2 spans */
  k = (int)(parc->height >> 1) + (int)((lw - 1) >> 1);
  if (spdata == (miArcSpanData *)NULL || spdata->k != k)
    {
      if (spdata)
	{
	  free (spdata->spans);
	  free (spdata);
	}
      spdata = (miArcSpanData *)mi_xmalloc (sizeof(miArcSpanData));
      spdata->spans = (miArcSpan *)mi_xmalloc ((k + 2) * sizeof (miArcSpan));
      spdata->k = k;		/* k+2 is size of empty span array */
      lruent->spdata = spdata;
    }
  lruent->lrustamp = ++(ellipseCache->lrustamp);	/* set timestamp, bump clock */
  lruent->lw = lw;
  lruent->width = parc->width;
  lruent->height = parc->height;
  if (lruent != &fakeent)	/* non-huge ellipse; store in cache */
    ellipseCache->lastCacheHit = lruent;

  /* compute spans, place them in the new cache record */
  if (parc->width == parc->height)
    miComputeCircleSpans (lw, parc, spdata);
  else
    miComputeEllipseSpans (lw, parc, spdata);
  return spdata;
}

/* Compute the spans that make up a complete wide circle, via a fast
   integer algorithm.  On entry, lw>=1, and `spdata' is a pointer to an
   miArcSpanData struct, which is a slot in a record in the ellipse span
   cache.  It includes spdata->spans, an array of k+2 ArcSpans, which will
   be filled in.  Here k=height/2 + (lw-1)/2. */
static void
#ifdef _HAVE_PROTOS
miComputeCircleSpans (unsigned int lw, const miArc *parc, miArcSpanData *spdata)
#else
miComputeCircleSpans (lw, parc, spdata)
     unsigned int lw;
     const miArc *parc;
     miArcSpanData *spdata;
#endif
{
  miArcSpan *span;
  int doinner;
  int x, y, e;
  int xk, yk, xm, ym, dx, dy;
  int slw, inslw;
  int inx = 0, iny, ine = 0;
  int inxk = 0, inyk = 0, inxm = 0, inym = 0;

  /* compute flags */
  /* top=true iff ellipse width is even and line width is even */
  spdata->top = !(lw & 1) && !(parc->width & 1) ? true : false;
  /* bot=true iff ellipse height is even, so there will be an _odd_
     number of spans, from top to bottom */
  spdata->bot = !(parc->height & 1) ? true : false;

  doinner = -(int)lw;
  slw = (int)parc->width - doinner;
  y = (int)(parc->height >> 1);
  dy = parc->height & 1;	/* dy=1 if height is odd */
  dx = 1 - dy;			/* dx=1 if height is even */
  MIWIDEARCSETUP(x, y, dy, slw, e, xk, xm, yk, ym);
  inslw = (int)parc->width + doinner;
  if (inslw > 0)
    {
      /* if top=true, will need to add an extra pixel (not included in the
         generated list of ArcSpans) in the `hole'; this is a kludge */
      spdata->hole = spdata->top;
      MIWIDEARCSETUP(inx, iny, dy, inslw, ine, inxk, inxm, inyk, inym);
    }
  else
    {
      spdata->hole = false;
      doinner = -y;
    }

  /* Generate the ArcSpans at successive values of y, beginning at the top
     of the circle and extending down to its horizontal bisector.  Also,
     fill in the count1/count2/top/bottom elements of the miArcSpanData
     struct pointed to by spdata.  There will be

     (top ? 1 : 0) + count1 + count2 + (bottom ? 1 : 0) 

     ArcSpans in all.  The first ones [(top ? 1 : 0) + count1 in number]
     will be single-occupied, i.e., they will include only one span.
     The latter ones [count2 + (bottom ? 1 : 0) in number]
     will be doubly-occupied, i.e., they will include two spans.

     For the special role of the very first and very last ArcSpans in the
     list, to fix which the `top' and `bottom' kludge flags were
     introduced, see following comments. */

  /* If top=true, first ArcSpan generated by the following `while' loop
     will be bogus, and will need to be replaced, when drawing, by a single
     point.  So decrement count1 to compensate. */
  spdata->count1 = -doinner - (spdata->top ? 1 : 0);
  spdata->count2 = y + doinner;

  span = spdata->spans;
  /* initial value of y is (width+lw)/2 + (1 if height is even) */
  while (y)
    {
      MIFILLARCSTEP(x, y, e, xk, xm, yk, ym, dx, slw); /* y-- */
      span->lx = dy - x;
      if (++doinner <= 0)
 	{
	  span->lw = slw;
	  span->rx = 0;
	  span->rw = span->lx + slw;
	}
      else
	{
	  MIFILLINARCSTEP(inx, iny, ine, inxk, inxm, inyk, inym, dx, inslw);
	  span->lw = x - inx;
	  span->rx = dy - inx + inslw;
	  span->rw = inx - x + slw - inslw;
	}
      span++;
    }

  if (spdata->bot)
    /* last-generated ArcSpan, on the horizontal center line, is special,
       so modify it and decrement count2 (or count1) to compensate */
    {
      if (spdata->count2 > 0)
	spdata->count2--;
      else
	/* no two-span ArcSpans at all; ellipse isn't hollow */
	{
	  if (lw > parc->height)
	    span[-1].rx = span[-1].rw = -(((int)lw - (int)parc->height) >> 1);
	  else
	    span[-1].rw = 0;
	  spdata->count1--;
	}
    }
}


/*
The following mathematics is the background for the algorithm used in
miComputeEllipseSpans() below, which scan-converts a wide ellipse.

The following three equations combine to describe the boundaries of a wide
ellipse, if it is drawn with a circular brush.

 x^2/w^2 + y^2/h^2 = 1			ellipse itself
 (X-x)^2 + (Y-y)^2 = r^2		circle at (x, y) on the ellipse
 (Y-y) = (X-x)*w^2*y/(h^2*x)		normal at (x, y) on the ellipse

These lead to a quartic relating Y and y

y^4 - (2Y)y^3 + (Y^2 + (h^4 - w^2*r^2)/(w^2 - h^2))y^2
    - (2Y*h^4/(w^2 - h^2))y + (Y^2*h^4)/(w^2 - h^2) = 0

The reducible cubic obtained from this quartic is

z^3 - (3N)z^2 - 2V = 0

where

N = (Y^2 + (h^4 - w^2*r^2/(w^2 - h^2)))/6
V = w^2*r^2*Y^2*h^4/(4 *(w^2 - h^2)^2)

Let

t = z - N
p = -N^2
q = -N^3 - V

Then we get

t^3 + 3pt + 2q = 0

The discriminant of this cubic is

D = q^2 + p^3

When D > 0, a real root is obtained as

z = N + cbrt(-q+sqrt(D)) + cbrt(-q-sqrt(D))

When D < 0, a real root is obtained as

z = N - 2m*cos(acos(-q/m^3)/3)

where

m = sqrt(|p|) * sign(q)

Given a real root Z of the cubic, the roots of the quartic are the roots
of the two quadratics

y^2 + ((b+A)/2)y + (Z + (bZ - d)/A) = 0

where 

A = +/- sqrt(8Z + b^2 - 4c)
b, c, d are the cubic, quadratic, and linear coefficients of the quartic

Some experimentation is then required to determine which solutions
correspond to the inner and outer boundaries of the wide ellipse.  */


/* Compute the spans that make up a complete wide ellipse, via a floating
   point algorithm motivated by the above math.  On entry, lw>=1, and
   `spdata' is a pointer to an miArcSpanData struct, which is a slot in a
   record in the ellipse span cache.  It includes spdata->spans, an array
   of k+2 ArcSpans, which will be filled in.  Here 
   k=height/2 + (lw-1)/2. */
static void
#ifdef _HAVE_PROTOS
miComputeEllipseSpans (unsigned int lw, const miArc *parc, miArcSpanData *spdata)
#else
miComputeEllipseSpans (lw, parc, spdata)
     unsigned int lw;
     const miArc *parc;
     miArcSpanData *spdata;
#endif
{
  miArcSpan *span;
  double w, h, r, xorg;
  double Hs, Hf, WH, K, Vk, Nk, Fk, Vr, N, Nc, Z, rs;
  double A, T, b, d, x, y, t, inx, outx = 0, hepp, hepm;
  int flip;
  bool solution;
  
  /* compute flags */
  /* top=true iff ellipse width is even and line width is even */
  spdata->top = !(lw & 1) && !(parc->width & 1) ? true : false;
  /* bot=true iff ellipse height is even, so there will be an _odd_
     number of spans, from top to bottom */
  spdata->bot = !(parc->height & 1) ? true : false;
  /* a kludge */
  spdata->hole = ((spdata->top 
		   && parc->height * lw <= parc->width * parc->width
		   && lw < parc->height) ? true : false);

  w = 0.5 * parc->width;
  h = 0.5 * parc->height;
  r = 0.5 * lw;
  rs = r * r;
  Hs = h * h;
  WH = w * w - Hs;
  Nk = w * r;
  Vk = (Nk * Hs) / (WH + WH);
  Hf = Hs * Hs;
  Nk = (Hf - Nk * Nk) / WH;
  Fk = Hf / WH;
  hepp = h + EPSILON;
  hepm = h - EPSILON;
  K = h + ((lw - 1) >> 1);
  if (parc->width & 1)
    xorg = .5;
  else
    xorg = 0.0;
  spdata->count1 = 0;
  spdata->count2 = 0;

  /* Generate list of spans, going downward from top of ellipse,
     i.e. more or less at y = yorgu = yorg_arc + height_arc/2 - k.
     Most of these will be mirrored, going upward from the bottom
     of the ellipse, starting at y = yorgu = yorg_arc + height_arc/2 + k. */
  span = spdata->spans;

  if (spdata->top)
    /* top=true if ellipse width is even and line width is even; if so,
       begin with a special (non-mirrored) ArcSpan containing a single `top
       point', at y=yorgu-1 */
    {
      span->lx = 0;
      span->lw = 1;
      span++;
    }

  /* generate count1 + count2 ArcSpans, at y=yorgu, y=yorgu+1,...;
     count1 one-span ArcSpans come first, then count2 two-span ArcSpans */
  for (; K > 0.0; K -= 1.0)
    {
      N = (K * K + Nk) / 6.0;
      Nc = N * N * N;
      Vr = Vk * K;
      t = Nc + Vr * Vr;
      d = Nc + t;
      if (d < 0.0) 
	{
	  d = Nc;
	  b = N;
	  if ( (b < 0.0) == (t < 0.0) )
	    {
	      b = -b;
	      d = -d;
	    }
	  Z = N - 2.0 * b * cos(acos(-t / d) / 3.0);
	  if ( (Z < 0.0) == (Vr < 0.0) )
	    flip = 2;
	  else
	    flip = 1;
	}
      else
	{
	  d = Vr * sqrt(d);
	  Z = N + cbrt(t + d) + cbrt(t - d);
	  flip = 0;
	}
      A = sqrt((Z + Z) - Nk);
      T = (Fk - Z) * K / A;
      inx = 0.0;
      solution = false;
      b = -A + K;
      d = b * b - 4 * (Z + T);
      if (d >= 0)
	{
	  d = sqrt(d);
	  y = 0.5 * (b + d);
	  if ((y >= 0.0) && (y < hepp))
	    {
	      solution = true;
	      if (y > hepm)
		y = h;
	      t = y / h;
	      x = w * sqrt(1 - (t * t));
	      t = K - y;
	      t = sqrt(rs - (t * t));
	      if (flip == 2)
		inx = x - t;
	      else
		outx = x + t;
	    }
	}
      b = A + K;
      d = b * b - 4 * (Z - T);
      /* Because of the large magnitudes involved, we lose enough precision
       * that sometimes we end up with a negative value near the axis, when
       * it should be positive.  This is a workaround.
       */
      if (d < 0 && !solution)
	d = 0.0;
      if (d >= 0) 
	{
	  d = sqrt(d);
	  y = 0.5 * (b + d);
	  if (y < hepp)
	    {
	      if (y > hepm)
		y = h;
	      t = y / h;
	      x = w * sqrt(1 - (t * t));
	      t = K - y;
	      inx = x - sqrt(rs - (t * t));
	    }
	  y = 0.5 * (b - d);
	  if (y >= 0.0)
	    {
	      if (y > hepm)
		y = h;
	      t = y / h;
	      x = w * sqrt(1 - (t * t));
	      t = K - y;
	      t = sqrt(rs - (t * t));
	      if (flip == 1)
		inx = x - t;
	      else
		outx = x + t;
	    }
	}
      span->lx = ICEIL(xorg - outx);
      if (inx <= 0.0)
	{
	  /* a one-span ArcSpan (they come first) */
	  spdata->count1++;
	  span->lw = ICEIL(xorg + outx) - span->lx;
	  span->rx = ICEIL(xorg + inx);
	  span->rw = -ICEIL(xorg - inx);
	}
      else
	{
	  /* a two-span ArcSpan (they come second) */
	  spdata->count2++;
	  span->lw = ICEIL(xorg - inx) - span->lx;
	  span->rx = ICEIL(xorg + inx);
	  span->rw = ICEIL(xorg + outx) - span->rx;
	}
      span++;
    }

  if (spdata->bot)
    /* bot=true if ellipse height is even; if so, complete the ellipse by
       adding a final ArcSpan at the horizontal center line, containing
       either two spans or one span (if the ellipse isn't hollow) */
    {
      outx = w + r;
      if (r >= h && r <= w)
	inx = 0.0;
      else if (Nk < 0.0 && -Nk < Hs)
	{
	  inx = w * sqrt(1 + Nk / Hs) - sqrt(rs + Nk);
	  if (inx > w - r)
	    inx = w - r;
	}
      else
	inx = w - r;
      span->lx = ICEIL(xorg - outx);
      if (inx <= 0.0)
	{
	  span->lw = ICEIL(xorg + outx) - span->lx;
	  span->rx = ICEIL(xorg + inx);
	  span->rw = -ICEIL(xorg - inx);
	}
      else
	{
	  span->lw = ICEIL(xorg - inx) - span->lx;
	  span->rx = ICEIL(xorg + inx);
	  span->rw = ICEIL(xorg + outx) - span->rx;
	}
    }

  if (spdata->hole)
    /* convert the final one-span ArcSpan to the initial two-span ArcSpan,
       so that there will be a one-pixel `hole' to be filled */
    {
      span = &spdata->spans[spdata->count1];
      span->lw = -span->lx;
      span->rx = 1;
      span->rw = span->lw;
      spdata->count1--;
      spdata->count2++;
    }
}


/**********************************************************************/
/* miComputeArcs() and miFreeArcs(), called by miPolyArc(). */
/**********************************************************************/

/* Compute arc segments, caps, and joins in a polyarc, taking account of
   dashing.  Return value is a list of miPolyArcs structs, indexed by pixel
   paint type, which will need to be freed with miFreeArcs().  If dashing,
   sub-pixel placement of arc segment endpoints will normally occur. */

static miPolyArcs *
#ifdef _HAVE_PROTOS
miComputeArcs (const miGC *pGC, const miArc *parcs, int narcs)
#else
miComputeArcs (pGC, parcs, narcs)
     const miGC *pGC;
     const miArc *parcs;
     int narcs;
#endif
{
  bool		isDashed, isDoubleDash;
  miPolyArcs	*arcs;
  int		i, start, k, nextk;
  miArcData	*data;
  int		numPixels;
  int		paintType, paintTypeStart, prevPaintType;
  int	        dashNum, dashIndex, dashRemaining;
  int		dashNumStart, dashIndexStart, dashRemainingStart;
  
  isDashed = (pGC->lineStyle == (int)MI_LINE_SOLID ? false : true);
  isDoubleDash = (pGC->lineStyle == (int)MI_LINE_DOUBLE_DASH ? true : false);
  numPixels = pGC->numPixels;
  
  /* allocate and initialize list of miPolyArcs that will be returned */
  arcs = (miPolyArcs *) mi_xmalloc (numPixels * sizeof(miPolyArcs));
  for (paintType = 0; paintType < numPixels; paintType++) 
    {
      arcs[paintType].arcs = (miArcData *)NULL;
      arcs[paintType].narcs = 0;
      arcs[paintType].arcSize = 0; /* slots allocated */
		
      arcs[paintType].caps = (miArcCapStruct *)NULL;
      arcs[paintType].ncaps = 0;
      arcs[paintType].capSize = 0; /* slots allocated */

      arcs[paintType].joins = (miArcJoinStruct *)NULL;
      arcs[paintType].njoins = 0;
      arcs[paintType].joinSize = 0; /* slots allocated */
    }

  /* allocate and fill temporary struct with starting point, ending point,
     self-join status of each elliptic arc */

#define todeg(xAngle)	(((double) (xAngle)) / 64.0)

  data = (miArcData *) mi_xmalloc (narcs * sizeof (miArcData));
  for (i = 0; i < narcs; i++) 
    {
      double a0, a1;
      int angle2;

      a0 = todeg (parcs[i].angle1);
      angle2 = parcs[i].angle2;
      if (angle2 > FULLCIRCLE)
	angle2 = FULLCIRCLE;
      else if (angle2 < -FULLCIRCLE)
	angle2 = -FULLCIRCLE;
      data[i].selfJoin = ((angle2 == FULLCIRCLE) || (angle2 == -FULLCIRCLE) 
			  ? true : false);
      a1 = todeg (parcs[i].angle1 + angle2);
      data[i].x0 = parcs[i].x + (double) parcs[i].width / 2*(1 + miDcos (a0));
      data[i].y0 = parcs[i].y + (double) parcs[i].height / 2*(1 - miDsin (a0));
      data[i].x1 = parcs[i].x + (double) parcs[i].width / 2*(1 + miDcos (a1));
      data[i].y1 = parcs[i].y + (double) parcs[i].height / 2*(1 - miDsin (a1));
    }
  
  /* initialize paint type and dashing state (latter is not used in `solid'
     case) */
  paintType = 1;
  dashNum = 0;
  dashIndex = 0;
  dashRemaining = 0;
  if (isDashed) 
    /* take offsetting into account */
    {
      int dashOffset = 0;

      /* alter paint type (for first dash) and dashing state */
      miStepDash (pGC->dashOffset, &dashNum, &dashIndex,
		  pGC->dash, pGC->numInDashList, &dashOffset);
      paintType = (dashNum & 1) ? 0 : 1 + ((dashNum / 2) % (numPixels - 1));
      dashRemaining = (int)(pGC->dash[dashIndex]) - dashOffset;	
    }
  /* save paint type and dashing state (will reset at each unjoined arc) */
  paintTypeStart = paintType;
  dashNumStart = dashNum;
  dashIndexStart = dashIndex;
  dashRemainingStart = dashRemaining;

  /* iterate backward over arcs; determine whether cap will be required
     after each arc, and stop when first such is seen */
  for (i = narcs - 1; i >= 0; i--) 
    {
      int j;

      j = i + 1;
      if (j == narcs)
	j = 0;
      if (data[i].selfJoin || i == j ||
	  (UNEQUAL (data[i].x1, data[j].x0) ||
	   UNEQUAL (data[i].y1, data[j].y0)))
	{
	  /* if starting in `on' phase, add a cap at right end */
	  if (paintType != 0 || isDoubleDash)
	    addCap (&arcs[paintType], RIGHT_END, 0);
	  break;
	}
    }

  /* iterate forward over all successive pairs of arcs (wrap if necessary) */

  start = i + 1;
  if (start == narcs)
    start = 0;
  i = start;
  k = nextk = 0;
  /* keep compiler happy by initting prevPaintType too; actually
     unnecessary because first thing drawn won't be a join */
  prevPaintType = paintType;
  
  for (;;) 
    {
      int j, nexti;
      miArcData *arc;
      bool arcsJoin;

      j = i + 1;
      if (j == narcs)
	j = 0;
      nexti = i + 1;
      if (nexti == narcs)
	nexti = 0;

      if (isDashed) 
	{
	  int		startAngle, spanAngle, endAngle;
	  int		dashAngle, prevDashAngle;
	  bool		backwards, selfJoin;
	  dashMap	map;
	  miArc		xarc;

	  /*
	   * precompute an approximation map for use in dashing
	   */
	  computeDashMap (&parcs[i], &map);
	  /*
	   * compute each individual dash segment using the path
	   * length function
	   */
	  startAngle = parcs[i].angle1;
	  spanAngle = parcs[i].angle2;
	  if (spanAngle > FULLCIRCLE)
	    spanAngle = FULLCIRCLE;
	  else if (spanAngle < -FULLCIRCLE)
	    spanAngle = -FULLCIRCLE;
	  if (startAngle < 0)
	    startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
	  if (startAngle >= FULLCIRCLE)
	    startAngle = startAngle % FULLCIRCLE;
	  endAngle = startAngle + spanAngle;
	  backwards = (spanAngle < 0 ? true : false);
	  dashAngle = startAngle;
	  selfJoin = (data[i].selfJoin && (paintType != 0 || isDoubleDash) 
		      ? true : false);
	  
	  /*
	   * add dashed arcs to each bucket
	   */
	  arc = (miArcData *)NULL;
	  while (dashAngle != endAngle) 
	    {
	      prevDashAngle = dashAngle;
	      dashAngle = computeAngleFromPath (prevDashAngle, endAngle, &map,
						&dashRemaining, backwards);
	      /* avoid troubles with huge arcs and small dashes */
	      if (dashAngle == prevDashAngle) 
		{
		  if (backwards)
		    dashAngle--;
		  else
		    dashAngle++;
		}
	      if (paintType != 0 || isDoubleDash) 
		{
		  xarc = parcs[i];
		  spanAngle = prevDashAngle;
		  if (spanAngle < 0)
		    spanAngle = FULLCIRCLE - (-spanAngle) % FULLCIRCLE;
		  if (spanAngle >= FULLCIRCLE)
		    spanAngle = spanAngle % FULLCIRCLE;
		  xarc.angle1 = spanAngle;
		  spanAngle = dashAngle - prevDashAngle;
		  if (backwards) 
		    {
		      if (dashAngle > prevDashAngle)
			spanAngle = - FULLCIRCLE + spanAngle;
		    } 
		  else 
		    {
		      if (dashAngle < prevDashAngle)
			spanAngle = FULLCIRCLE + spanAngle;
		    }
		  if (spanAngle > FULLCIRCLE)
		    spanAngle = FULLCIRCLE;
		  if (spanAngle < -FULLCIRCLE)
		    spanAngle = -FULLCIRCLE;
		  xarc.angle2 = spanAngle;
		  arc = addArc (&arcs[paintType], &xarc);
		  /*
		   * cap each end of an on/off dash
		   */
		  if (!isDoubleDash)
		    {
		      if (prevDashAngle != startAngle) 
			addCap (&arcs[paintType],
				RIGHT_END, arc - arcs[paintType].arcs);
		      if (dashAngle != endAngle) 
			addCap (&arcs[paintType],
				LEFT_END, arc - arcs[paintType].arcs);
		    }
		  arc->cap = arcs[paintType].ncaps;
		  arc->join = arcs[paintType].njoins;
		  arc->render = false;
		  arc->selfJoin = false;
		  if (dashAngle == endAngle)
		    arc->selfJoin = selfJoin;
		}
	      prevPaintType = paintType;
	      if (dashRemaining <= 0) 
		/* on to next dash (negative means overshoot due to
		   rounding; positive means undershoot due to rounding, in
		   which case we don't bump dashNum or dashIndex, or toggle
		   the dash phase: next dash will have same paint type */
		{
		  dashNum++;
		  dashIndex++;
		  if (dashIndex == pGC->numInDashList) /* wrap */
		    dashIndex = 0;
		  /* recompute paintType, dashRemaining for next dash */
		  paintType = 
		    (dashNum & 1) ? 0 : 1 + ((dashNum / 2) % (numPixels - 1));
		  dashRemaining = (int)(pGC->dash[dashIndex]);
		}
	    }
	  /*
	   * make sure a place exists for the position data if
	   * we drew (i.e. didn't draw) a zero-length arc
	   */
	  if (startAngle == endAngle) /* zero-length */
	    {
	      prevPaintType = paintType;
	      if (isDoubleDash == false && paintType == 0)
		/* use color of most recent `on' dash */
		{
		  if (dashNum == 0)
		    prevPaintType = numPixels - 1;
		  else		/* can use infix % operator */
		    prevPaintType = 
		      ((dashNum - 1) & 1) ? 0 : 1 + (((dashNum - 1)/ 2) % (numPixels - 1));
		}
	      arc = addArc (&arcs[prevPaintType], &parcs[i]);
	      arc->join = arcs[prevPaintType].njoins;
	      arc->cap = arcs[prevPaintType].ncaps;
	      arc->selfJoin = data[i].selfJoin;
	    }
	}
      else
	/* not dashing; just add whole (solid) arc */
	{
	  arc = addArc (&arcs[paintType], &parcs[i]);
	  arc->join = arcs[paintType].njoins;
	  arc->cap = arcs[paintType].ncaps;
	  arc->selfJoin = data[i].selfJoin;
	  prevPaintType = paintType;
	}

      if (prevPaintType != 0 || isDoubleDash)
	k = arcs[prevPaintType].narcs - 1;
      if (paintType != 0 || isDoubleDash)
	nextk = arcs[paintType].narcs;

      if (nexti == start) 
	{
	  nextk = 0;
	  if (isDashed) 
	    /* re-initialize paint type and dashing state */
	    {
	      paintType = paintTypeStart;
	      dashNum = dashNumStart;
	      dashIndex = dashIndexStart;
	      dashRemaining = dashRemainingStart;
	    }
	}

      /* does the successive pair of arcs join? */
      arcsJoin = (narcs > 1 && i != j 
		  && ISEQUAL (data[i].x1, data[j].x0)
		  && ISEQUAL (data[i].y1, data[j].y0)
		  && data[i].selfJoin == false 
		  && data[j].selfJoin == false) ? true : false;
      if (arc != (miArcData *)NULL)
	{
	  if (arcsJoin)
	    arc->render = false;
	  else
	    /* no join; so add directive to render first arc */
	    arc->render = true;
	}
      if (arcsJoin
	  && (prevPaintType != 0 || isDoubleDash) 
	  && (paintType != 0 || isDoubleDash))
	/* arcs join, and both are `on' */
	{
	  int joinPaintType;

	  joinPaintType = paintType;
	  if (isDoubleDash) 
	    {
	      if (nexti == start)
		joinPaintType = paintTypeStart;
	      /* if join is right at the dash and there are two colors to
		 choose from, draw join in a foreground color */
	      if (joinPaintType == 0)
		{
		  if (prevPaintType != 0)
		    joinPaintType = prevPaintType;
		  else  /* shouldn't happen; just use next dash's color */
		    joinPaintType =
		      ((dashNum + 1) & 1) ? 0 : 1 + (((dashNum + 1)/ 2) % (numPixels - 1));
		}
	    }
	  if (joinPaintType != 0 || isDoubleDash) 
	    {
	      addJoin (&arcs[joinPaintType],
		       LEFT_END, k, prevPaintType,
		       RIGHT_END, nextk, paintType);
	      arc->join = arcs[prevPaintType].njoins;
	    }
	}
      else 
	/* arcs don't join (or if they do, at least one is `off') */
	{
	  /*
	   * cap the left end of this arc
	   * unless it joins itself
	   */
	  if ((prevPaintType != 0 || isDoubleDash)
	      && arc->selfJoin == false)
	    {
	      addCap (&arcs[prevPaintType], LEFT_END, k);
	      arc->cap = arcs[prevPaintType].ncaps;
	    }
	  if (isDashed && arcsJoin == false)
	    /* re-initialize paint type and dashing state */
	    {
	      paintType = paintTypeStart;
	      dashNum = dashNumStart;
	      dashIndex = dashIndexStart;
	      dashRemaining = dashRemainingStart;
	    }
	  nextk = arcs[paintType].narcs;
	  if (nexti == start) 
	    {
	      nextk = 0;
	      /* re-initialize paint type and dashing state */
	      paintType = paintTypeStart;
	      dashNum = dashNumStart;
	      dashIndex = dashIndexStart;
	      dashRemaining = dashRemainingStart;
	    }
	  /*
	   * cap the right end of the next arc.  If the
	   * next arc is actually the first arc, only
	   * cap it if it joins with this arc.  This
	   * case will occur when the final dash segment
	   * of an on/off dash is off.  Of course, this
	   * cap will be drawn at a strange time, but that
	   * hardly matters...
	   */
	  if ((paintType != 0 || isDoubleDash)
	      && (nexti != start || (arcsJoin && isDashed)))
	    addCap (&arcs[paintType], RIGHT_END, nextk);
	}

      i = nexti;
      if (i == start)
	/* have now iterated over all successive pairs (cyclically) */
	break;
    }

   /* make sure the last arc if any (i.e. miArcData struct) in each
      paint-type-specific miPolyArcs struct includes a `render' directive */
  for (paintType = 0; paintType < numPixels; paintType++)
    if (arcs[paintType].narcs > 0) 
      {
	arcs[paintType].arcs[arcs[paintType].narcs-1].render = true;
	arcs[paintType].arcs[arcs[paintType].narcs-1].join =
	  arcs[paintType].njoins;
	arcs[paintType].arcs[arcs[paintType].narcs-1].cap =
	  arcs[paintType].ncaps;
      }

  free (data);

  /* return the array of paint-type-specific miPolyArcs structs */
  return arcs;
}

/* Free a list of arc segments (i.e. dashes) for an incomplete ellipse,
   indexed by pixel paint type, that was computed by miComputeArcs(). */
static void
#ifdef _HAVE_PROTOS
miFreeArcs(const miGC *pGC, miPolyArcs *arcs)
#else
miFreeArcs(pGC, arcs)
     const miGC *pGC;
     miPolyArcs *arcs;
#endif
{
  int paintType;

  for (paintType = 0; paintType < pGC->numPixels; paintType++)
    {
      if (arcs[paintType].narcs > 0)
	free (arcs[paintType].arcs);
      if (arcs[paintType].njoins > 0)
	free (arcs[paintType].joins);
      if (arcs[paintType].ncaps > 0)
	free (arcs[paintType].caps);
    }
  free (arcs);
}


/**********************************************************************/
/* addCap(), addJoin(), addArc().  These three helper functions are used by
   miComputeArcs(). */
/**********************************************************************/

#define ADD_REALLOC_STEP	20

/* helper function called by miComputeArcs(); add a cap to the array of
   miArcCapStructs in a miPolyArcs struct */
static void
#ifdef _HAVE_PROTOS
addCap (miPolyArcs *polyArcs, int end, int arcIndex)
#else
addCap (polyArcs, end, arcIndex)
     miPolyArcs *polyArcs;
     int end;
     int arcIndex;
#endif
{
  miArcCapStruct *cap;

  if (polyArcs->ncaps == polyArcs->capSize)
    /* expand array */
    {
      int newsize = polyArcs->capSize + ADD_REALLOC_STEP;
      miArcCapStruct *newcaps;

      newcaps = (miArcCapStruct *) mi_xrealloc (polyArcs->caps,
					     newsize * sizeof(miArcCapStruct));
      polyArcs->caps = newcaps;
      polyArcs->capSize = newsize;
    }

  cap = &(polyArcs->caps[polyArcs->ncaps]);
  cap->end = end;
  cap->arcIndex = arcIndex;
  ++(polyArcs->ncaps);
}

/* helper function called by miComputeArcs(); add a join to the array of
   miArcJoinStructs in a miPolyArcs struct */
static void
#ifdef _HAVE_PROTOS
addJoin (miPolyArcs *polyArcs, int end0, int index0, int paintType0, int end1, int index1, int paintType1)
#else
addJoin (polyArcs, end0, index0, paintType0, end1, index1, paintType1)
     miPolyArcs *polyArcs;
     int end0, index0, paintType0;
     int end1, index1, paintType1;
#endif
{
  miArcJoinStruct *join;

  if (polyArcs->njoins == polyArcs->joinSize)
    /* expand array */
    {
      int newsize = polyArcs->joinSize + ADD_REALLOC_STEP;
      miArcJoinStruct *newjoins;

      newjoins = (miArcJoinStruct *) mi_xrealloc (polyArcs->joins,
					    newsize * sizeof(miArcJoinStruct));
      polyArcs->joins = newjoins;
      polyArcs->joinSize = newsize;
    }

  join = &(polyArcs->joins[polyArcs->njoins]);
  join->end0 = end0;
  join->arcIndex0 = index0;
  join->paintType0 = paintType0;
  join->end1 = end1;
  join->arcIndex1 = index1;
  join->paintType1 = paintType1;
  ++(polyArcs->njoins);
}

/* helper function called by miComputeArcs(); add a arc (i.e. an miArc) to
   the array of miArcData structs in a miPolyArcs struct, and return a
   pointer to the new miArcData struct */
static miArcData *
#ifdef _HAVE_PROTOS
addArc (miPolyArcs *polyArcs, const miArc *xarc)
#else
addArc (polyArcs, xarc)
     miPolyArcs *polyArcs;
     const miArc *xarc;
#endif
{
  miArcData *arc;

  if (polyArcs->narcs == polyArcs->arcSize)
    /* expand array */
    {
      int newsize = polyArcs->arcSize + ADD_REALLOC_STEP;
      miArcData *newarcs;

      newarcs = (miArcData *) mi_xrealloc (polyArcs->arcs,
					   newsize * sizeof(miArcData));
      polyArcs->arcs = newarcs;
      polyArcs->arcSize = newsize;
    }

  arc = &(polyArcs->arcs[polyArcs->narcs]);
  arc->arc = *xarc;
  ++(polyArcs->narcs);

  return arc;
}

/**********************************************************************/
/* miArcJoin() and miArcCap().  These two low-level functions are called by
   miPolyArc().  They draw joins and caps by calling miFillSppPoly(), which
   calls the low-level paint function.  */
/**********************************************************************/

/* Draw a join between two contiguous arcs, by calling miFillSppPoly(). */
static void
#ifdef _HAVE_PROTOS
miArcJoin (miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, const miArcFace *pLeft, const miArcFace *pRight, int xOrgLeft, int yOrgLeft, double xFtransLeft, double yFtransLeft, int xOrgRight, int yOrgRight, double xFtransRight, double yFtransRight)
#else
miArcJoin (paintedSet, pixel, pGC, pLeft, pRight, xOrgLeft, yOrgLeft, xFtransLeft, yFtransLeft, xOrgRight, yOrgRight, xFtransRight, yFtransRight)
     miPaintedSet *paintedSet;
     miPixel pixel;
     const miGC *pGC;
     const miArcFace *pLeft, *pRight;
     int xOrgLeft, yOrgLeft;
     double xFtransLeft, yFtransLeft;
     int xOrgRight, yOrgRight;
     double xFtransRight, yFtransRight;
#endif
{
  SppPoint	center, corner, otherCorner;
  SppPoint	poly[5];
  SppPoint	*pArcPts;
  int		cpt;
  SppArc	arc;
  miArcFace	Right, Left;
  int		polyLen = 0;
  int		xOrg, yOrg;
  double	xFtrans, yFtrans;
  double	a;
  double	width;
  double	halftheta;
	
  xOrg = (xOrgRight + xOrgLeft) / 2;
  yOrg = (yOrgRight + yOrgLeft) / 2;
  xFtrans = (xFtransLeft + xFtransRight) / 2;
  yFtrans = (yFtransLeft + yFtransRight) / 2;
  Right = *pRight;
  translateBounds (&Right, xOrg - xOrgRight, yOrg - yOrgRight,
		   xFtrans - xFtransRight, yFtrans - yFtransRight);
  Left = *pLeft;
  translateBounds (&Left, xOrg - xOrgLeft, yOrg - yOrgLeft,
		   xFtrans - xFtransLeft, yFtrans - yFtransLeft);
  pRight = &Right;
  pLeft = &Left;

  if (pRight->clock.x == pLeft->counterClock.x
      && pRight->clock.y == pLeft->counterClock.y)
    return;

  /* determine corners of cap */
  center = pRight->center;
  if (0 <= (a = angleBetween (center, pRight->clock, pLeft->counterClock))
      && a <= 180.0)
    {
      corner = pRight->clock;
      otherCorner = pLeft->counterClock;
    } 
  else		/* interchange to make a <= 180, we hope */
    {
      a = angleBetween (center, pLeft->clock, pRight->counterClock);
      corner = pLeft->clock;
      otherCorner = pRight->counterClock;
    }

  width = (pGC->lineWidth ? pGC->lineWidth : 1);
  switch (pGC->joinStyle) 
    {
    case MI_JOIN_MITER:
    default:
      /* miter only if MITERLIMIT * sin(theta/2) >= 1.0,
	 where theta = 180-a is the join angle */

      if ((halftheta = 0.5 * (180.0 - a)) > 0.0
	  && miDsin (halftheta) * pGC->miterLimit >= 1.0)
	/* miter limit not exceeded */
	{
	  double ae, ac2, ec2, bc2, de;
	  SppPoint e;
	  
	  /* miter, i.e. add quadrilateral */
	  poly[0] = corner;
	  poly[1] = center;
	  poly[2] = otherCorner;
	  bc2 = ((corner.x - otherCorner.x) * (corner.x - otherCorner.x) +
		 (corner.y - otherCorner.y) * (corner.y - otherCorner.y));
	  ec2 = 0.25 * bc2;
	  ac2 = ((corner.x - center.x) * (corner.x - center.x) +
		 (corner.y - center.y) * (corner.y - center.y));
	  ae = sqrt (ac2 - ec2);
	  de = ec2 / ae;
	  e.x = 0.5 * (corner.x + otherCorner.x);
	  e.y = 0.5 * (corner.y + otherCorner.y);
	  poly[3].x = e.x + de * (e.x - center.x) / ae;
	  poly[3].y = e.y + de * (e.y - center.y) / ae;
	  poly[4] = corner;
	  polyLen = 5;
	}
      else			/* miter limit exceeded */
	{
	  /* bevel, i.e. add triangle */
	  poly[0] = corner;
	  poly[1] = center;
	  poly[2] = otherCorner;
	  poly[3] = corner;
	  polyLen = 4;
	}
      miFillSppPoly (paintedSet, pixel,
		     polyLen, poly, xOrg, yOrg, xFtrans, yFtrans);
      break;
    case MI_JOIN_BEVEL:
      /* add triangle */
      poly[0] = corner;
      poly[1] = center;
      poly[2] = otherCorner;
      poly[3] = corner;
      polyLen = 4;
      miFillSppPoly (paintedSet, pixel,
		     polyLen, poly, xOrg, yOrg, xFtrans, yFtrans);
      break;
    case MI_JOIN_TRIANGULAR:
      /* add stubby quadrilateral */
      {
	double mid2, mid;
	SppPoint e;
	
	e.x = 0.5 * (corner.x + otherCorner.x);
	e.y = 0.5 * (corner.y + otherCorner.y);
	mid2 = ((e.x - center.x) * (e.x - center.x) +
		(e.y - center.y) * (e.y - center.y));
	mid = sqrt (mid2);
	
	poly[0] = corner;
	poly[1] = center;
	poly[2] = otherCorner;
	poly[3].x = e.x + 0.5 * width * (e.x - center.x) / mid;
	poly[3].y = e.y + 0.5 * width * (e.y - center.y) / mid;
	poly[4] = corner;
	polyLen = 5;
	miFillSppPoly (paintedSet, pixel,
		       polyLen, poly, xOrg, yOrg, xFtrans, yFtrans);
      }
      break;
    case MI_JOIN_ROUND:
      /* add round cap */
      arc.x = center.x - width/2;
      arc.y = center.y - width/2;
      arc.width = width;
      arc.height = width;
      arc.angle1 = -miDatan2 (corner.y - center.y, corner.x - center.x);
      arc.angle2 = a;

      pArcPts = (SppPoint *) mi_xmalloc (3 * sizeof (SppPoint));
      pArcPts[0] = otherCorner;
      pArcPts[1] = center;
      pArcPts[2] = corner;
      /* convert semicircle to a polyline, and fill */
      if ((cpt = miGetArcPts (&arc, 3, &pArcPts)))
	/* by drawing with miFillSppPoly and setting the endpoints of the
	   arc to be the corners, we ensure that the cap will meet up with
	   the rest of the line */
	miFillSppPoly (paintedSet, pixel,
		       cpt, pArcPts, xOrg, yOrg, xFtrans, yFtrans);
      free (pArcPts);
      break;
    }
}

/* helper function, used by miArcJoin() above */
static double
#ifdef _HAVE_PROTOS
angleBetween (SppPoint center, SppPoint point1, SppPoint point2)
#else
angleBetween (center, point1, point2)
     SppPoint center, point1, point2;
#endif
{
  double	a1, a2, a;
	
  /*
   * reflect from X coordinates back to ellipse
   * coordinates -- y increasing upwards
   */
  a1 = miDatan2 (- (point1.y - center.y), point1.x - center.x);
  a2 = miDatan2 (- (point2.y - center.y), point2.x - center.x);
  a = a2 - a1;
  if (a <= -180.0)
    a += 360.0;
  else if (a > 180.0)
    a -= 360.0;

  return a;
}

/* helper function, used by miArcJoin() above */
static void
#ifdef _HAVE_PROTOS
translateBounds (miArcFace *b, int x, int y, double fx, double fy)
#else
translateBounds (b, x, y, fx, fy)
     miArcFace *b;
     int x, y;
     double fx, fy;
#endif
{
  fx += x;
  fy += y;
  b->clock.x -= fx;
  b->clock.y -= fy;
  b->center.x -= fx;
  b->center.y -= fy;
  b->counterClock.x -= fx;
  b->counterClock.y -= fy;
}

/* Draw a cap on an arc segment, by calling miFillSppPoly(). */
/*ARGSUSED*/
static void
#ifdef _HAVE_PROTOS
miArcCap (miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, const miArcFace *pFace, int end, int xOrg, int yOrg, double xFtrans, double yFtrans)
#else
miArcCap (paintedSet, pixel, pGC, pFace, end, xOrg, yOrg, xFtrans, yFtrans)
     miPaintedSet *paintedSet;
     miPixel pixel;
     const miGC *pGC;
     const miArcFace *pFace;
     int end;
     int xOrg, yOrg;
     double xFtrans, yFtrans;
#endif
{
  SppPoint corner, otherCorner, center, endPoint, poly[5];

  corner = pFace->clock;
  otherCorner = pFace->counterClock;
  center = pFace->center;
  switch (pGC->capStyle) 
    {
    case MI_CAP_BUTT:
    case MI_CAP_NOT_LAST:
    default:
      break;			/* do nothing */
    case MI_CAP_PROJECTING:
      poly[0].x = otherCorner.x;
      poly[0].y = otherCorner.y;
      poly[1].x = corner.x;
      poly[1].y = corner.y;
      poly[2].x = corner.x - (center.y - corner.y);
      poly[2].y = corner.y + (center.x - corner.x);
      poly[3].x = otherCorner.x - (otherCorner.y - center.y);
      poly[3].y = otherCorner.y + (otherCorner.x - center.x);
      poly[4].x = otherCorner.x;
      poly[4].y = otherCorner.y;
      miFillSppPoly (paintedSet, pixel,
		     5, poly, xOrg, yOrg, xFtrans, yFtrans);
      break;
    case MI_CAP_TRIANGULAR:
      poly[0].x = otherCorner.x;
      poly[0].y = otherCorner.y;
      poly[1].x = corner.x;
      poly[1].y = corner.y;
      poly[2].x = center.x - 0.5 * (otherCorner.y - corner.y);
      poly[2].y = center.y + 0.5 * (otherCorner.x - corner.x);
      poly[3].x = otherCorner.x;
      poly[3].y = otherCorner.y;
      miFillSppPoly (paintedSet, pixel,
		     4, poly, xOrg, yOrg, xFtrans, yFtrans);
      break;
    case MI_CAP_ROUND:
      /*
       * miRoundCap() just needs these to be unequal.
       */
      endPoint = center;
      endPoint.x = endPoint.x + 100;
      miRoundCap (paintedSet, pixel, pGC,
		  center, endPoint, corner, otherCorner, 0,
		  -xOrg, -yOrg, xFtrans, yFtrans);
      break;
    }
}

/* MIROUNDCAP -- a helper function used by miArcCap() above.
 * Put Rounded cap on end.  pCenter is the center of this end of the line
 * pEnd is the center of the other end of the line.  pCorner is one of the
 * two corners at this end of the line.  
 * NOTE:  pOtherCorner must be counter-clockwise from pCorner.
 */
/*ARGSUSED*/
static void
#ifdef _HAVE_PROTOS
miRoundCap (miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, SppPoint pCenter, SppPoint pEnd, SppPoint pCorner, SppPoint pOtherCorner, int fLineEnd, int xOrg, int yOrg, double xFtrans, double yFtrans)
#else
miRoundCap (paintedSet, pixel, pGC, pCenter, pEnd, pCorner, pOtherCorner, fLineEnd, xOrg, yOrg, xFtrans, yFtrans)
     miPaintedSet *paintedSet;
     miPixel pixel;
     const miGC *pGC;
     SppPoint pCenter, pEnd, pCorner, pOtherCorner;
     int fLineEnd;
     int xOrg, yOrg;
     double xFtrans, yFtrans;
#endif
{
  int		cpt;
  double	width;
  SppArc	arc;
  SppPoint	*pArcPts;

  width = (pGC->lineWidth ? pGC->lineWidth : 1);

  arc.x = pCenter.x - width/2;
  arc.y = pCenter.y - width/2;
  arc.width = width;
  arc.height = width;
  arc.angle1 = -miDatan2 (pCorner.y - pCenter.y, pCorner.x - pCenter.x);
  if (PTISEQUAL(pCenter, pEnd))
    arc.angle2 = - 180.0;
  else 
    {
      arc.angle2 = -miDatan2 (pOtherCorner.y - pCenter.y, pOtherCorner.x - pCenter.x) - arc.angle1;
      if (arc.angle2 < 0)
	arc.angle2 += 360.0;
    }
  
  /* convert semicircle to a polyline, and fill */
  pArcPts = (SppPoint *)NULL;
  if ((cpt = miGetArcPts (&arc, 0, &pArcPts)))
    /* by drawing with miFillSppPoly and setting the endpoints of the arc
     * to be the corners, we assure that the cap will meet up with the
     * rest of the line */
    miFillSppPoly (paintedSet, pixel,
		   cpt, pArcPts, -xOrg, -yOrg, xFtrans, yFtrans);
  free (pArcPts);
}

/* MIGETARCPTS -- Converts an arc into a set of line segments, so the
 * resulting polygon can be filled -- a helper routine for drawing round
 * joins and caps.  Returns the number of points in the arc.  Note that it
 * takes a pointer to a pointer to where it should put the points and an
 * index (cpt).  This procedure allocates the space necessary to fit the
 * arc points.  Sometimes it's convenient for those points to be at the end
 * of an existing array. (For example, if we want to leave a spare point to
 * make sectors instead of segments.)  So we pass in the malloc'ed chunk
 * that contains the array, and an index saying where we should start
 * stashing the points.  If there isn't an array already, we just pass in a
 * null pointer and count on mi_xrealloc() to handle the null pointer
 * correctly.  */
static int
#ifdef _HAVE_PROTOS
miGetArcPts (const SppArc *parc, int cpt, SppPoint **ppPts)
#else
miGetArcPts (parc, cpt, ppPts)
     const SppArc *parc;
     int cpt;			/* number of points already in arc list */
     SppPoint **ppPts;		/* ptr to ptr to arc-list -- modified */
#endif
{
  double st;			/* Start Theta, start angle */
  double et;			/* End Theta, offset from start theta */
  double dt;			/* Delta Theta, angle to sweep ellipse */
  double cdt;			/* Cos Delta Theta, actually 2 cos(dt) */
  double x0, y0;		/* recurrence formula needs 2 points to start*/
  double x1, y1;
  double x2, y2;		/* this will be the new point generated */
  double xc, yc;		/* the center point */
  int count, i;
  SppPoint *poly;
  miPoint last;			/* last point on integer boundaries */

  /* The spec says that positive angles indicate counterclockwise motion.
     Given our coordinate system (with 0,0 in the upper left corner), the
     screen appears flipped in Y.  The easiest fix is to negate the angles
     given. */
  st = - parc->angle1;
  et = - parc->angle2;

  /* Try to get a delta theta that is within 1/2 pixel.  Then adjust it
   * so that it divides evenly into the total.
   * I'm just using cdt 'cause I'm lazy.
   */
  cdt = parc->width;
  if (parc->height > cdt)
    cdt = parc->height;
  cdt *= 0.5;
  if (cdt <= 0)
    return 0;
  if (cdt < 1.0)
    cdt = 1.0;
  dt = miDasin (1.0 / cdt);	/* minimum step necessary */
  count = (int)(et/dt);
  count = abs(count) + 1;
  dt = et/count;	
  count++;

  cdt = 2 * miDcos(dt);
  poly = (SppPoint *) mi_xrealloc(*ppPts,
				     (cpt + count) * sizeof(SppPoint));
  *ppPts = poly;

  xc = 0.5 * parc->width;	/* store half width and half height */
  yc = 0.5 * parc->height;
    
  x0 = xc * miDcos(st);
  y0 = yc * miDsin(st);
  x1 = xc * miDcos(st + dt);
  y1 = yc * miDsin(st + dt);
  xc += parc->x;		/* by adding initial point, these become */
  yc += parc->y;		/* the center point */

  poly[cpt].x = (xc + x0);
  poly[cpt].y = (yc + y0);
  poly[cpt + 1].x = (xc + x1);
  poly[cpt + 1].y = (yc + y1);
  last.x = IROUND(xc + x1);
  last.y = IROUND(yc + y1);

  for (i = 2; i < count; i++)
    {
      x2 = cdt * x1 - x0;
      y2 = cdt * y1 - y0;

      poly[cpt + i].x = (xc + x2);
      poly[cpt + i].y = (yc + y2);

      x0 = x1; y0 = y1;
      x1 = x2; y1 = y2;
    }
  /* adjust the last point */
  if (FABS(parc->angle2) >= 360.0)
    poly[cpt +i -1] = poly[0];
  else
    {
      poly[cpt +i -1].x = (miDcos(st + et) * 0.5 * parc->width + xc);
      poly[cpt +i -1].y = (miDsin(st + et) * 0.5 * parc->height + yc);
    }

  return count;
}


/**********************************************************************/
/* Specially defined trig functions.  At the cardinal points, they are
   exact. */
/**********************************************************************/

#define Dsin(d)	((d) == 0.0 ? 0.0 : ((d) == 90.0 ? 1.0 : sin(d*M_PI/180.0)))
#define Dcos(d)	((d) == 0.0 ? 1.0 : ((d) == 90.0 ? 0.0 : cos(d*M_PI/180.0)))
#define mod(a,b)	((a) >= 0 ? (a) % (b) : (b) - (-a) % (b))

static double
#ifdef _HAVE_PROTOS
miDcos (double a)
#else
miDcos (a)
     double a;
#endif
{
  int	i;

  if (floor (a/90) == a/90) 
    {
      i = (int) (a/90.0);
      switch (mod (i, 4)) 
	{
	case 0: return 1;
	case 1: return 0;
	case 2: return -1;
	case 3: return 0;
	}
    }
  return cos (a * M_PI / 180.0);
}

static double
#ifdef _HAVE_PROTOS
miDsin (double a)
#else
miDsin (a)
     double a;
#endif
{
  int	i;

  if (floor (a/90) == a/90) 
    {
      i = (int) (a/90.0);
      switch (mod (i, 4)) 
	{
	case 0: return 0;
	case 1: return 1;
	case 2: return 0;
	case 3: return -1;
	}
    }
  return sin (a * M_PI / 180.0);
}

static double
#ifdef _HAVE_PROTOS
miDasin (double v)
#else
miDasin (v)
     double v;
#endif
{
  if (v == 0)
    return 0.0;
  if (v == 1.0)
    return 90.0;
  if (v == -1.0)
    return -90.0;
  return asin(v) * (180.0 / M_PI);
}

static double 
#ifdef _HAVE_PROTOS
miDatan2 (double dy, double dx)
#else
miDatan2 (dy, dx)
     double dy, dx;
#endif
{
  if (dy == 0) 
    {
      if (dx >= 0)
	return 0.0;
      return 180.0;
    } 
  else if (dx == 0) 
    {
      if (dy > 0)
	return 90.0;
      return -90.0;
    } 
  else if (FABS(dy) == FABS(dx)) 
    {
      if (dy > 0) 
	{
	  if (dx > 0)
	    return 45.0;
	  return 135.0;
	} 
      else 
	{
	  if (dx > 0)
	    return 315.0;
	  return 225.0;
	}
    } 
  else 
    return atan2 (dy, dx) * (180.0 / M_PI);
}


/***********************************************************************/
/* A sub-module that computes arc lengths via a polygonal approximation to
 * the arc.  External functions are computeDashMap(), which should be
 * called first, and the primary function computeAngleFromPath().  They are
 * called by miComputeArcs() above.  */
/***********************************************************************/

#define dashIndexToAngle(di)	((((double) (di)) * 90.0) / ((double) DASH_MAP_SIZE - 1))
#define xAngleToDashIndex(xa)	((((long) (xa)) * (DASH_MAP_SIZE - 1)) / (90 * 64))
#define dashIndexToXAngle(di)	((((long) (di)) * (90 * 64)) / (DASH_MAP_SIZE - 1))
#define dashXAngleStep	(((double) (90 * 64)) / ((double) (DASH_MAP_SIZE - 1)))

/* forward references (functions in this sub-module) */
static double angleToLength ____P((int angle, const dashMap *map));
static int lengthToAngle ____P((double len, const dashMap *map));

static void
#ifdef _HAVE_PROTOS
computeDashMap (const miArc *arcp, dashMap *map)
#else
computeDashMap (arcp, map)
     const miArc *arcp;
     dashMap *map;
#endif
{
  int di;
  double a, x, y, prevx = 0.0, prevy = 0.0, dist;

  for (di = 0; di < DASH_MAP_SIZE; di++) 
    {
      a = dashIndexToAngle (di);
      x = (double)(0.5 * arcp->width) * miDcos (a);
      y = (double)(0.5 * arcp->height) * miDsin (a);
      if (di == 0) 
	map->map[di] = 0.0;
      else 
	{
	  dist = hypot (x - prevx, y - prevy);
	  map->map[di] = map->map[di - 1] + dist;
	}
      prevx = x;
      prevy = y;
    }
}

static double
#ifdef _HAVE_PROTOS
angleToLength (int angle, const dashMap *map)
#else
angleToLength (angle, map)
     int angle;
     const dashMap *map;
#endif
{
  double len, excesslen, sidelen = map->map[DASH_MAP_SIZE - 1], totallen;
  int    di;
  int	 excess;
  bool	 oddSide = false;

  totallen = 0;
  if (angle >= 0) 
    {
      while (angle >= 90 * 64) 
	{
	  angle -= 90 * 64;
	  totallen += sidelen;
	  oddSide = (oddSide ? false : true);
	}
    } 
  else 
    {
      while (angle < 0) 
	{
	  angle += 90 * 64;
	  totallen -= sidelen;
	  oddSide = (oddSide ? false : true);
	}
    }
  if (oddSide)
    angle = 90 * 64 - angle;
		
  di = xAngleToDashIndex (angle);
  excess = angle - dashIndexToXAngle (di);

  len = map->map[di];
  /*
   * linearly interpolate between this point and the next
   */
  if (excess > 0) 
    {
      excesslen = (map->map[di + 1] - map->map[di]) *
	((double) excess) / dashXAngleStep;
      len += excesslen;
    }
  if (oddSide)
    totallen += (sidelen - len);
  else
    totallen += len;
  return totallen;
}

/*
 * len is along the arc, but may be more than one rotation
 */

static int
#ifdef _HAVE_PROTOS
lengthToAngle (double len, const dashMap *map)
#else
lengthToAngle (len, map)
     double len;
     const dashMap *map;
#endif
{
  double sidelen = map->map[DASH_MAP_SIZE - 1];
  int angle, angleexcess;
  bool oddSide = false;
  int a0, a1, a;

  angle = 0;
  /*
   * step around the ellipse, subtracting sidelens and
   * adding 90 degrees.  oddSide will tell if the
   * map should be interpolated in reverse
   */
  if (len >= 0) 
    {
      if (sidelen == 0)
	return 2 * FULLCIRCLE;	/* infinity */
      while (len >= sidelen) 
	{
	  angle += 90 * 64;
	  len -= sidelen;
	  oddSide = (oddSide ? false : true);
	}
    } 
  else 
    {
      if (sidelen == 0)
	return -2 * FULLCIRCLE;	/* infinity */
      while (len < 0) 
	{
	  angle -= 90 * 64;
	  len += sidelen;
	  oddSide = (oddSide ? false : true);
	}
    }
  if (oddSide)
    len = sidelen - len;
  a0 = 0;
  a1 = DASH_MAP_SIZE - 1;
  /*
   * binary search for the closest pre-computed length
   */
  while (a1 - a0 > 1) 
    {
      a = (a0 + a1) / 2;
      if (len > map->map[a])
	a0 = a;
      else
	a1 = a;
    }
  angleexcess = dashIndexToXAngle (a0);
  /*
   * linearly interpolate to the next point
   */
  angleexcess += (int)((len - map->map[a0]) /
		       (map->map[a0+1] - map->map[a0]) * dashXAngleStep);
  if (oddSide)
    angle += (90 * 64) - angleexcess;
  else
    angle += angleexcess;
  return angle;
}

/* Compute the subtended angle, in 1/64 degree units, of an elliptic arc
 * that corresponds to a specified dash length.  The correct solution to
 * this problem involves an elliptic integral, so we punt by approximating
 * (it's only for dashes anyway...).  The approximation uses a polygon.
 *
 * The specified dash length `len' is updated, to equal the amount of the
 * dash that will remain after drawing the arc.  This may be nonzero due to
 * rounding.  The new value will be negative if the arc extends beyond the
 * specified dash length, and positive if the specified dash length extends
 * beyond the arc.  */

static int
#ifdef _HAVE_PROTOS
computeAngleFromPath (int startAngle, int endAngle, const dashMap *map, int *lenp, bool backwards)
/* start, endAngle are angles in 1/64 degree units */
#else
computeAngleFromPath (startAngle, endAngle, map, lenp, backwards)
     int startAngle, endAngle;
     const dashMap *map;
     int *lenp;
     bool backwards;
#endif
{
  int	a0, a1, a;
  double len0;
  int	len;

  a0 = startAngle;
  a1 = endAngle;
  len = *lenp;
  if (backwards) 
    /* flip the problem around to be forwards */
    {
      a0 = FULLCIRCLE - a0;
      a1 = FULLCIRCLE - a1;
    }

  if (a1 < a0)
    a1 += FULLCIRCLE;
  len0 = angleToLength (a0, map);
  a = lengthToAngle (len0 + len, map);
  if (a > a1) 
    {
      a = a1;
      len = (int)(len - angleToLength (a1, map) - len0);
    } 
  else
    len = 0;
  if (backwards)
    a = FULLCIRCLE - a;

  *lenp = len;
  return a;
}


/***********************************************************************/
/* Geometry computations related to wide ellipses, e.g., computeAcc(),
   which computes `accelerators' (frequently used quantities), and
   computeBounds(). */
/***********************************************************************/

/* definition of a wide arc */
struct arc_def 
{
  double	w, h;		/* half-width, half-height */
  double	l;		/* half of line width */
  double	a0, a1;		/* start angle, and angle range */
};

struct bound 
{
  double min, max;
};

struct ibound 
{
  int min, max;
};

/*
 * These are all y value bounds; computed by computeBounds().
 */
struct arc_bound 
{
  struct bound ellipse;
  struct bound inner, outer;
  struct bound right, left;
  struct ibound inneri, outeri;
};

struct line 
{
  double m, b;			/* for y = mx + b */
  bool valid;
};

/* Quantities frequently used when drawn an ellipse or elliptic arc;
   computed by computeAcc(). */
struct accelerators 
{
  double		tail_y;	/* "y value associated with bottom of tail" */
  double		h2;	/* half-height squared */
  double		w2;	/* half-width squared */
  double		h4;	/* half-height raised to 4th power */
  double		w4;	/* half-width raised to 4th power */
  double		h2mw2;	/* h2 minus w2 */
  double		h2l;	/* h2 times l (i.e. half the line width) */
  double		w2l;	/* w2 times l (i.e. half the line width) */
  double		fromIntX; /* 0.5 if width is odd, otherwise 0.0 */
  double		fromIntY; /* 0.5 if height is oddd, otherwise 0.0 */
  struct line		left, right;
  int			yorgu;
  int			yorgl;
  int			xorg;
};

#define boundedLe(value, bounds)\
	((bounds).min <= (value) && (value) <= (bounds).max)

#define intersectLine(y,line) (line.m * (y) + line.b)

/* forward references */
static double hookEllipseY ____P((double scan_y, const struct arc_bound *bound, const struct accelerators *acc, bool left));
static double hookX ____P((double scan_y, const struct arc_def *def, const struct arc_bound *bound, const struct accelerators *acc, bool left));
static double innerXfromXY ____P((double x, double y, const struct accelerators *acc));
static double innerYfromXY ____P((double x, double y, const struct accelerators *acc));
static double innerYfromY ____P((double y, const struct arc_def *def, const struct accelerators *acc));
static double outerXfromXY ____P((double x, double y, const struct accelerators *acc));
static double outerYfromXY ____P((double x, double y, const struct accelerators *acc));
static double tailX ____P((double K, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc));
static void computeAcc ____P((const miArc *tarc, unsigned int lw, struct arc_def *def, struct accelerators *acc));
static void computeBound ____P((const struct arc_def *def, struct arc_bound *bound, struct accelerators *acc, miArcFace *right, miArcFace *left));
static void computeLine ____P((double x1, double y1, double x2, double y2, struct line *line));
static void tailEllipseY ____P((const struct arc_def *def, struct accelerators *acc));

static double
#ifdef _HAVE_PROTOS
tailX (double K, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc)
#else
tailX (K, def, bounds, acc)
     double K;
     const struct arc_def *def;
     const struct arc_bound *bounds;
     const struct accelerators *acc;
#endif
{
  double w, h, r;
  double Hs, Hf, WH, Vk, Nk, Fk, Vr, N, Nc, Z, rs;
  double A, T, b, d, x, y, t, hepp, hepm;
  int flip, solution;
  double xs[2];
  double *xp;
  
  w = def->w;
  h = def->h;
  r = def->l;
  rs = r * r;
  Hs = acc->h2;
  WH = -acc->h2mw2;
  Nk = def->w * r;
  Vk = (Nk * Hs) / (WH + WH);
  Hf = acc->h4;
  Nk = (Hf - Nk * Nk) / WH;
  if (K == 0.0) 
    {
      if (Nk < 0.0 && -Nk < Hs) 
	{
	  xs[0] = w * sqrt(1 + Nk / Hs) - sqrt(rs + Nk);
	  xs[1] = w - r;
	  if (acc->left.valid && boundedLe(K, bounds->left) &&
	      !boundedLe(K, bounds->outer) && xs[0] >= 0.0 && xs[1] >= 0.0)
	    return xs[1];
	  if (acc->right.valid && boundedLe(K, bounds->right) &&
	      !boundedLe(K, bounds->inner) && xs[0] <= 0.0 && xs[1] <= 0.0)
	    return xs[1];
	  return xs[0];
	}
      return w - r;
    }
  Fk = Hf / WH;
  hepp = h + EPSILON;
  hepm = h - EPSILON;
  N = (K * K + Nk) / 6.0;
  Nc = N * N * N;
  Vr = Vk * K;
  xp = xs;
  xs[0] = 0.0;
  t = Nc + Vr * Vr;
  d = Nc + t;
  if (d < 0.0) 
    {
      d = Nc;
      b = N;
      if ( (b < 0.0) == (t < 0.0) )
	{
	  b = -b;
	  d = -d;
	}
      Z = N - 2.0 * b * cos (acos (-t / d) / 3.0);
      if ( (Z < 0.0) == (Vr < 0.0) )
	flip = 2;
      else
	flip = 1;
    }
  else
    {
      d = Vr * sqrt (d);
      Z = N + cbrt (t + d) + cbrt (t - d);
      flip = 0;
    }
  A = sqrt ((Z + Z) - Nk);
  T = (Fk - Z) * K / A;
  solution = false;
  b = -A + K;
  d = b * b - 4 * (Z + T);
  if (d >= 0 && flip == 2)
    {
      d = sqrt(d);
      y = 0.5 * (b + d);
      if ((y >= 0.0) && (y < hepp))
	{
	  solution = true;
	  if (y > hepm)
	    y = h;
	  t = y / h;
	  x = w * sqrt(1 - (t * t));
	  t = K - y;
	  t = sqrt(rs - (t * t));
	  *xp++ = x - t;
	}
    }
  b = A + K;
  d = b * b - 4 * (Z - T);
  /* Because of the large magnitudes involved, we lose enough precision
   * that sometimes we end up with a negative value near the axis, when
   * it should be positive.  This is a workaround.
   */
  if (d < 0 && !solution)
    d = 0.0;
  if (d >= 0) 
    {
      d = sqrt(d);
      y = 0.5 * (b + d);
      if (y < hepp)
	{
	  if (y > hepm)
	    y = h;
	  t = y / h;
	  x = w * sqrt(1 - (t * t));
	  t = K - y;
	  *xp++ = x - sqrt(rs - (t * t));
	}
      y = 0.5 * (b - d);
      if (y >= 0.0 && flip == 1)
	{
	  if (y > hepm)
	    y = h;
	  t = y / h;
	  x = w * sqrt(1 - (t * t));
	  t = K - y;
	  t = sqrt(rs - (t * t));
	  *xp++ = x - t;
	}
    }
  if (xp > &xs[1]) 
    {
      if (acc->left.valid && boundedLe(K, bounds->left) &&
	  !boundedLe(K, bounds->outer) && xs[0] >= 0.0 && xs[1] >= 0.0)
	return xs[1];
      if (acc->right.valid && boundedLe(K, bounds->right) &&
	  !boundedLe(K, bounds->inner) && xs[0] <= 0.0 && xs[1] <= 0.0)
	return xs[1];
    }
  return xs[0];
}

/*
 * This computes the ellipse y value associated with the
 * bottom of the tail.
 */

#define CUBE_ROOT_2	1.2599210498948732038115849718451499938964
#define CUBE_ROOT_4	1.5874010519681993173435330390930175781250

static void
#ifdef _HAVE_PROTOS
tailEllipseY (const struct arc_def *def, struct accelerators *acc)
#else
tailEllipseY (def, acc)
     const struct arc_def *def;
     struct accelerators *acc;
#endif
{
  double t;

  acc->tail_y = 0.0;
  if (def->w == def->h)
    return;
  t = def->l * def->w;
  if (def->w > def->h) 
    {
      if (t < acc->h2)
	return;
    } 
  else 
    {
      if (t > acc->h2)
	return;
    }
  t = 2.0 * def->h * t;
  t = (CUBE_ROOT_4 * acc->h2 - cbrt(t * t)) / acc->h2mw2;
  if (t > 0.0)
    acc->tail_y = def->h / CUBE_ROOT_2 * sqrt(t);
}

/*
 * inverse functions -- compute edge coordinates
 * from the ellipse (actually, from its precomputed accelerators)
 */

static double
#ifdef _HAVE_PROTOS
outerXfromXY (double x, double y, const struct accelerators *acc)
#else
outerXfromXY (x, y, acc)
     double x, y;
     const struct accelerators *acc;
#endif
{
  return x + (x * acc->h2l) / sqrt (x*x * acc->h4 + y*y * acc->w4);
}

static double
#ifdef _HAVE_PROTOS
outerYfromXY (double x, double y, const struct accelerators *acc)
#else
outerYfromXY (x, y, acc)
     double x, y;
     const struct accelerators *acc;
#endif
{
  return y + (y * acc->w2l) / sqrt (x*x * acc->h4 + y*y * acc->w4);
}

static double
#ifdef _HAVE_PROTOS
innerXfromXY (double x, double y, const struct accelerators *acc)
#else
innerXfromXY (x, y, acc)
     double x, y;
     const struct accelerators *acc;
#endif
{
  return x - (x * acc->h2l) / sqrt (x*x * acc->h4 + y*y * acc->w4);
}

static double
#ifdef _HAVE_PROTOS
innerYfromXY (double x, double y, const struct accelerators *acc)
#else
innerYfromXY (x, y, acc)
     double x, y;
     const struct accelerators *acc;
#endif
{
  return y - (y * acc->w2l) / sqrt (x*x * acc->h4 + y*y * acc->w4);
}

static double
#ifdef _HAVE_PROTOS
innerYfromY (double y, const struct arc_def *def, const struct accelerators *acc)
#else
innerYfromY (y, def, acc)
     double y;
     const struct arc_def *def;
     const struct accelerators *acc;
#endif
{
  double x;
										 x = (def->w / def->h) * sqrt (acc->h2 - y*y);
										 return y - (y * acc->w2l) / sqrt (x*x * acc->h4 + y*y * acc->w4);
									       }
     
/* compute a line through two points */
static void
#ifdef _HAVE_PROTOS
computeLine (double x1, double y1, double x2, double y2, struct line *line)
#else
computeLine (x1, y1, x2, y2, line)
     double x1, y1, x2, y2;
     struct line *line;
#endif
{
  if (y1 == y2)
    line->valid = false;
  else 
    {
      line->m = (x1 - x2) / (y1 - y2);
      line->b = x1  - y1 * line->m;
      line->valid = true;
    }
}

/* Compute accelerators for an ellipse.  These are simply values that are
   used repeatedly in the computations.  Also begin filling in the arc_def
   structure too. */
static void
#ifdef _HAVE_PROTOS
computeAcc (const miArc *tarc, unsigned int lw, struct arc_def *def, struct accelerators *acc)
#else
computeAcc (tarc, lw, def, acc)
     const miArc *tarc;
     unsigned int lw;
     struct arc_def *def;
     struct accelerators *acc;
#endif
{
  def->w = 0.5 * (double)tarc->width;
  def->h = 0.5 * (double)tarc->height;
  def->l = 0.5 * (double)lw;
  acc->h2 = def->h * def->h;
  acc->w2 = def->w * def->w;
  acc->h4 = acc->h2 * acc->h2;
  acc->w4 = acc->w2 * acc->w2;
  acc->h2l = acc->h2 * def->l;
  acc->w2l = acc->w2 * def->l;
  acc->h2mw2 = acc->h2 - acc->w2;
  acc->fromIntX = (tarc->width & 1) ? 0.5 : 0.0;
  acc->fromIntY = (tarc->height & 1) ? 0.5 : 0.0;
  acc->xorg = tarc->x + (int)(tarc->width >> 1);
  acc->yorgu = tarc->y + (int)(tarc->height >> 1);
  acc->yorgl = acc->yorgu + (tarc->height & 1);
  tailEllipseY (def, acc);	/* fill in tail_y element of acc */
}
		
/* Compute y value bounds of various portions of the arc, the outer edge,
   the ellipse and the inner edge.  Also invoke computeLine to compute left
   and right lines (stored in accelerator structure). */
static void
#ifdef _HAVE_PROTOS
computeBound (const struct arc_def *def, struct arc_bound *bound, struct accelerators *acc, miArcFace *right, miArcFace *left)
#else
computeBound (def, bound, acc, right, left)
     const struct arc_def *def;
     struct arc_bound *bound;
     struct accelerators *acc;
     miArcFace *right, *left;
#endif
{
  double		t;
  double		innerTaily;
  double		tail_y;
  struct bound	innerx, outerx;
  struct bound	ellipsex;

  bound->ellipse.min = Dsin (def->a0) * def->h;
  bound->ellipse.max = Dsin (def->a1) * def->h;
  if (def->a0 == 45 && def->w == def->h)
    ellipsex.min = bound->ellipse.min;
  else
    ellipsex.min = Dcos (def->a0) * def->w;
  if (def->a1 == 45 && def->w == def->h)
    ellipsex.max = bound->ellipse.max;
  else
    ellipsex.max = Dcos (def->a1) * def->w;
  bound->outer.min = outerYfromXY (ellipsex.min, bound->ellipse.min, acc);
  bound->outer.max = outerYfromXY (ellipsex.max, bound->ellipse.max, acc);
  bound->inner.min = innerYfromXY (ellipsex.min, bound->ellipse.min, acc);
  bound->inner.max = innerYfromXY (ellipsex.max, bound->ellipse.max, acc);

  outerx.min = outerXfromXY (ellipsex.min, bound->ellipse.min, acc);
  outerx.max = outerXfromXY (ellipsex.max, bound->ellipse.max, acc);
  innerx.min = innerXfromXY (ellipsex.min, bound->ellipse.min, acc);
  innerx.max = innerXfromXY (ellipsex.max, bound->ellipse.max, acc);
	
  /* Save the line end points for the cap code to use.  Careful here, these
   * are in Cartesian coordinates (y increasing upwards) while the cap code
   * uses inverted coordinates (y increasing downwards).
   */

  if (right) 
    {
      right->counterClock.y = bound->outer.min;
      right->counterClock.x = outerx.min;
      right->center.y = bound->ellipse.min;
      right->center.x = ellipsex.min;
      right->clock.y = bound->inner.min;
      right->clock.x = innerx.min;
    }

  if (left) 
    {
      left->clock.y = bound->outer.max;
      left->clock.x = outerx.max;
      left->center.y = bound->ellipse.max;
      left->center.x = ellipsex.max;
      left->counterClock.y = bound->inner.max;
      left->counterClock.x = innerx.max;
    }

  bound->left.min = bound->inner.max;
  bound->left.max = bound->outer.max;
  bound->right.min = bound->inner.min;
  bound->right.max = bound->outer.min;

  computeLine (innerx.min, bound->inner.min, outerx.min, bound->outer.min,
	       &acc->right);
  computeLine (innerx.max, bound->inner.max, outerx.max, bound->outer.max,
	       &acc->left);

  if (bound->inner.min > bound->inner.max) 
    {
      t = bound->inner.min;
      bound->inner.min = bound->inner.max;
      bound->inner.max = t;
    }
  tail_y = acc->tail_y;
  if (tail_y > bound->ellipse.max)
    tail_y = bound->ellipse.max;
  else if (tail_y < bound->ellipse.min)
    tail_y = bound->ellipse.min;
  innerTaily = innerYfromY (tail_y, def, acc);
  if (bound->inner.min > innerTaily)
    bound->inner.min = innerTaily;
  if (bound->inner.max < innerTaily)
    bound->inner.max = innerTaily;
  bound->inneri.min = ICEIL(bound->inner.min - acc->fromIntY);
  bound->inneri.max = IFLOOR(bound->inner.max - acc->fromIntY);
  bound->outeri.min = ICEIL(bound->outer.min - acc->fromIntY);
  bound->outeri.max = IFLOOR(bound->outer.max - acc->fromIntY);
}

/*
 * this section computes the x value of the span at y 
 * intersected with the specified face of the ellipse.
 *
 * this is the min/max X value over the set of normal
 * lines to the entire ellipse,  the equation of the
 * normal lines is:
 *
 *     ellipse_x h^2                   h^2
 * x = ------------ y + ellipse_x (1 - --- )
 *     ellipse_y w^2                   w^2
 *
 * compute the derivative with-respect-to ellipse_y and solve
 * for zero:
 *    
 *       (w^2 - h^2) ellipse_y^3 + h^4 y
 * 0 = - ----------------------------------
 *       h w ellipse_y^2 sqrt (h^2 - ellipse_y^2)
 *
 *             (   h^4 y     )
 * ellipse_y = ( ----------  ) ^ (1/3)
 *             ( (h^2 - w^2) )
 *
 * The other two solutions to the equation are imaginary.
 *
 * This gives the position on the ellipse which generates
 * the normal with the largest/smallest x intersection point.
 *
 * Now compute the second derivative to check whether
 * the intersection is a minimum or maximum:
 *
 *    h (y0^3 (w^2 - h^2) + h^2 y (3y0^2 - 2h^2))
 * -  -------------------------------------------
 *          w y0^3 (sqrt (h^2 - y^2)) ^ 3
 *
 * as we only care about the sign,
 *
 * - (y0^3 (w^2 - h^2) + h^2 y (3y0^2 - 2h^2))
 *
 * or (to use accelerators),
 *
 * y0^3 (h^2 - w^2) - h^2 y (3y0^2 - 2h^2) 
 *
 */

/* Compute the position on the ellipse whose normal line intersects the
   given scan line maximally. */
static double
#ifdef _HAVE_PROTOS
hookEllipseY (double scan_y, const struct arc_bound *bound, const struct accelerators *acc, bool left)
#else
hookEllipseY (scan_y, bound, acc, left)
     double scan_y;
     const struct arc_bound *bound;
     const struct accelerators *acc;
     bool left;
#endif
{
  double ret;

  if (acc->h2mw2 == 0) 
    {
      if ( (scan_y > 0 && (left ? false : true)) || (scan_y < 0 && left) )
	return bound->ellipse.min;
      return bound->ellipse.max;
    }
  ret = (acc->h4 * scan_y) / (acc->h2mw2);
  if (ret >= 0)
    return cbrt (ret);
  else
    return -cbrt (-ret);
}

/* Compute the X value of the intersection of the given scan line with the
   right side of the lower hook. */
static double
#ifdef _HAVE_PROTOS
hookX (double scan_y, const struct arc_def *def, const struct arc_bound *bound, const struct accelerators *acc, bool left)
#else
hookX (scan_y, def, bound, acc, left)
     double scan_y;
     const struct arc_def *def;
     const struct arc_bound *bound;
     const struct accelerators *acc;
     bool left;
#endif
{
  double	ellipse_y, x;
  double	maxMin;

  if (def->w != def->h) 
    {
      ellipse_y = hookEllipseY (scan_y, bound, acc, left);
      if (boundedLe (ellipse_y, bound->ellipse)) 
	{
	  /*
	   * compute the value of the second
	   * derivative
	   */
	  maxMin = ellipse_y*ellipse_y*ellipse_y * acc->h2mw2 -
	    acc->h2 * scan_y * (3 * ellipse_y*ellipse_y - 2*acc->h2);
	  if ((left && maxMin > 0) || ((left ? false : true) && maxMin < 0))
	    {
	      if (ellipse_y == 0)
		return def->w + left ? -def->l : def->l;
	      x = (acc->h2 * scan_y - ellipse_y * acc->h2mw2) *
		sqrt (acc->h2 - ellipse_y * ellipse_y) /
		  (def->h * def->w * ellipse_y);
	      return x;
	    }
	}
    }
  if (left) 
    {
      if (acc->left.valid && boundedLe (scan_y, bound->left)) 
	x = intersectLine (scan_y, acc->left);
      else 
	{
	  if (acc->right.valid)
	    x = intersectLine (scan_y, acc->right);
	  else
	    x = def->w - def->l;
	}
    } 
  else 
    {
      if (acc->right.valid && boundedLe (scan_y, bound->right)) 
	x = intersectLine (scan_y, acc->right);
      else 
	{
	  if (acc->left.valid)
	    x = intersectLine (scan_y, acc->left);
	  else
	    x = def->w - def->l;
	}
    }
  return x;
}


/**********************************************************************/
/* The following three sub-modules, taken together, provide only five
   public functions: initAccumSpans(), which initializes an miAccumSpans
   structure, newFinalSpan(), which draws a single span to a miAccumSpans
   structure, drawArc(), which draws a single arc to a miAccumSpans
   structure as a collection of spans, drawZeroArc(), which draws a single
   degenerate (horizontal or vertical) arc, and finally fillSpans(), which
   paints the miAccumSpans structure, deallocates the spans, and resets the
   structure. */
/**********************************************************************/

/**********************************************************************/
/* A sub-module that accumulates an in-core cache of spans and on request,
   paints them.  Only two public functions are newFinalSpan() and
   fillSpans().  Former is invoked by the succeeding sub-module, which
   draws arcs as spans and in turn is invoked by the drawArc() sub-module.
   Latter is invoked above, in miPolyArc(), to clean things up. */
/**********************************************************************/

/* ???!!! a ceiling on amount by which finalSpans array is expanded !!!??? */
#define SPAN_REALLOC	100

/* forward references */
static struct finalSpan * realAllocSpan ____P((miAccumSpans *accumSpans));
static struct finalSpan ** realFindSpan ____P((miAccumSpans *accumSpans, int y));
static void disposeFinalSpans ____P((miAccumSpans *accumSpans));
static void newFinalSpan ____P((miAccumSpans *accumSpans, int y, int xmin, int xmax));

/*** allocation-related functions ***/

/* A public function for this module: initialize an miAccumSpans structure
   (an in-core accumulation of spans, which is added to by newFinalSpan(),
   and painted and deallocated by fillSpans()). */
static void
#ifdef _HAVE_PROTOS
initAccumSpans (miAccumSpans *accumSpans)
#else
initAccumSpans (accumSpans)
     miAccumSpans *accumSpans;
#endif
{
  accumSpans->finalSpans = (struct finalSpan **)NULL;
  accumSpans->finalMiny = 0;
  accumSpans->finalMaxy = -1;
  accumSpans->finalSize = 0;
  accumSpans->nspans = 0;
  accumSpans->chunks = (struct finalSpanChunk *)NULL;
  accumSpans->freeFinalSpans = (struct finalSpan *)NULL;
}

/* A public function for this module: add a span to an miAccumSpans
   structure.  By convention, span is [xmin, xmax-1] in terms of pixels.
   This agrees with the libxmi convention that `right edges' (as well as
   bottom edges) of polygons should be omitted, so that adjacent polygons
   can abut with no overlaps or gaps. */
static void
#ifdef _HAVE_PROTOS
newFinalSpan (miAccumSpans *accumSpans, int y, int xmin, int xmax)
#else
newFinalSpan (accumSpans, y, xmin, xmax)
     miAccumSpans *accumSpans;
     int y, xmin, xmax;
#endif
{
  struct finalSpan *x, *oldx, *prev, **f;

  /* find list of spans at this value of y in finalSpans array; if y isn't
     in the range finalMiny..finalMaxy, invoke realFindSpan() to expand
     finalSpans array */
  if (accumSpans->finalMiny <= y && y <= accumSpans->finalMaxy)
    f = &((accumSpans->finalSpans)[(y) - (accumSpans->finalMiny)]);
  else
    f = realFindSpan (accumSpans, y);

  /* loop through spans at y, trying to expand an existing one */
  if (f == (struct finalSpan **)NULL)
    return;
  oldx = (struct finalSpan *)NULL;
  for (;;) 
    {
      prev = (struct finalSpan *)NULL;
      for (x = *f; x; x = x->next) 
	{
	  if (x == oldx) 
	    {
	      prev = x;
	      continue;
	    }
	  if (x->min <= xmax && xmin <= x->max) 
	    /* expand span */
	    {
	      if (oldx) 
		{
		  oldx->min = IMIN (x->min, xmin);
		  oldx->max = IMAX (x->max, xmax);
		  if (prev)
		    prev->next = x->next;
		  else
		    *f = x->next;
		  --(accumSpans->nspans);
		} 
	      else 
		{
		  x->min = IMIN (x->min, xmin);
		  x->max = IMAX (x->max, xmax);
		  oldx = x;
		}
	      xmin = oldx->min;
	      xmax = oldx->max;
	      break;
	    }
	  prev = x;
	}
      if (!x)
	break;
    }

  if (!oldx) 
    /* couldn't expand an existing span at this value of y, so create a new
       one and add it to the list */
    {
      /* obtain new span from current chunk; if chunk is exhausted, invoke
	 realAllocSpan() to allocate a new one */
      if (accumSpans->freeFinalSpans != (struct finalSpan *)NULL)
	{
	  x = accumSpans->freeFinalSpans;
	  accumSpans->freeFinalSpans = accumSpans->freeFinalSpans->next;
	  x->next = (struct finalSpan *)NULL;
	}
      else
	x = realAllocSpan (accumSpans);

      if (x)
	{
	  x->min = xmin;
	  x->max = xmax;
	  x->next = *f;
	  *f = x;
	  ++(accumSpans->nspans);
	}
    }
}

/* Reallocate the finalSpans array in an miAccumSpans structure to include
   the specified value y.  This is called only if y is outside the range
   finalMiny..finalMaxy, which indexes the array.  Returns the address, in
   the finalSpans array, of the pointer to the head of the list of spans at
   the new value of y. */
static struct finalSpan **
#ifdef _HAVE_PROTOS
realFindSpan (miAccumSpans *accumSpans, int y)
#else
realFindSpan (accumSpans, y)
     miAccumSpans *accumSpans;
     int y;
#endif
{
  struct finalSpan	**newSpans, **t;
  int			newSize, newMiny, newMaxy;
  int			change;
  int			i, k;

  if (y < accumSpans->finalMiny || y > accumSpans->finalMaxy) 
    /* need to expand... */
    {
      if (accumSpans->finalSize == 0)
	{
	  accumSpans->finalMiny = y;
	  accumSpans->finalMaxy = y - 1;
	}
      if (y < accumSpans->finalMiny)
	change = accumSpans->finalMiny - y;
      else
	change = y - accumSpans->finalMaxy;

      /* ???!!! a ceiling on amount by which finalSpans is expanded !!!??? */
      if (change >= SPAN_REALLOC)
	change += SPAN_REALLOC;
      else
	change = SPAN_REALLOC;

      newSize = accumSpans->finalSize + change;

      newSpans = 
	(struct finalSpan **)mi_xmalloc (newSize * sizeof (struct finalSpan *));
      newMiny = accumSpans->finalMiny;
      newMaxy = accumSpans->finalMaxy;
      if (y < accumSpans->finalMiny)
	newMiny = accumSpans->finalMiny - change;
      else
	newMaxy = accumSpans->finalMaxy + change;
      if (accumSpans->finalSpans)
	{
	  memmove ((voidptr_t)(newSpans + (accumSpans->finalMiny - newMiny)),
		   (voidptr_t)(accumSpans->finalSpans),
		   accumSpans->finalSize * sizeof(struct finalSpan *));
	  free (accumSpans->finalSpans);
	}

      if ((i = accumSpans->finalMiny - newMiny) > 0)
	for (k = 0, t = newSpans; k < i; k++, t++)
	  *t = (struct finalSpan *)NULL;
      if ((i = newMaxy - accumSpans->finalMaxy) > 0)
	for (k = 0, t = newSpans + newSize - i; k < i; k++, t++)
	  *t = (struct finalSpan *)NULL;
      accumSpans->finalSpans = newSpans;
      accumSpans->finalMaxy = newMaxy;
      accumSpans->finalMiny = newMiny;
      accumSpans->finalSize = newSize;
    }

  return &((accumSpans->finalSpans)[(y) - (accumSpans->finalMiny)]);
}

/* Return an unused span, by allocating a new chunk of spans and returning
   the first span in the chunk.  Called only if freeFinalSpans pointer in
   the miAccumSpans structure is NULL, i.e., previously allocated chunk (if
   any) is exhausted.  The freeFinalSpans and chunks pointers are
   updated. */
static struct finalSpan *
#ifdef _HAVE_PROTOS
realAllocSpan (miAccumSpans *accumSpans)
#else
realAllocSpan (accumSpans)
     miAccumSpans *accumSpans;
#endif
{
  struct finalSpanChunk	*newChunk;
  struct finalSpan	*span;
  int			i;

  /* allocate new chunk, add to head of chunk list */
  newChunk = (struct finalSpanChunk *) mi_xmalloc (sizeof (struct finalSpanChunk));
  newChunk->next = accumSpans->chunks;
  accumSpans->chunks = newChunk;

  /* point freeFinalSpans to the second span in the new chunk */
  accumSpans->freeFinalSpans = newChunk->data + 1;

  /* be sure `next' pointer of each span in the new chunk is NULL */
  span = newChunk->data + 1;
  for (i = 1; i < SPAN_CHUNK_SIZE - 1; i++) 
    {
      span->next = span + 1;
      span++;
    }
  span->next = (struct finalSpan *)NULL;

  span = newChunk->data;
  span->next = (struct finalSpan *)NULL;
  return span;
}

/*** deallocation-related functions ***/

/* A public function for this module: paint spans that have been
   accumulated in an miAccumSpans structure, in a specified pixel color;
   also reset the structure, as if initAccumSpans() had been called.
   Painting takes place to the specified miPaintedSet structure, by
   invoking MI_PAINT_SPANS(). */

/* All painting done in this file goes through this function. */

static void
#ifdef _HAVE_PROTOS
fillSpans (miPaintedSet *paintedSet, miPixel pixel, miAccumSpans *accumSpans)
#else
fillSpans (paintedSet, pixel, accumSpans)
     miPaintedSet *paintedSet;
     miPixel pixel;
     miAccumSpans *accumSpans;
#endif
{
  struct finalSpan	*span;
  struct finalSpan	**f;
  int			spany;
  miPoint		*ppt, *pptInit;
  unsigned int		*pwidth, *pwidthInit;

  if (accumSpans->nspans == 0)
    return;

  /* from the miAccumSpans struct, construct an array of spans */
  ppt = pptInit = (miPoint *) mi_xmalloc (accumSpans->nspans * sizeof (miPoint));
  pwidth = pwidthInit = (unsigned int *) mi_xmalloc (accumSpans->nspans * sizeof (unsigned int));

  for (spany = accumSpans->finalMiny, f = accumSpans->finalSpans; 
       spany <= accumSpans->finalMaxy; 
       spany++, f++) 
    {
      for (span = *f; span; span = span->next) 
	{
	  if (span->max <= span->min)
	    continue;
	  ppt->x = span->min;
	  ppt->y = spany;
	  ++ppt;
	  *pwidth++ = (unsigned int)(span->max - span->min);
	}
    }

  /* paint the spans to the miPaintedSet */
  MI_PAINT_SPANS(paintedSet, pixel, ppt - pptInit, pptInit, pwidthInit)

  /* free all spans in the miAccumSpans struct, reset it */
  disposeFinalSpans (accumSpans);
  accumSpans->finalMiny = 0;
  accumSpans->finalMaxy = -1;
  accumSpans->finalSize = 0;
  accumSpans->nspans = 0;
}

static void
#ifdef _HAVE_PROTOS
disposeFinalSpans (miAccumSpans *accumSpans)
#else
disposeFinalSpans (accumSpans)
     miAccumSpans *accumSpans;
#endif
{
  struct finalSpanChunk	*chunk, *next;

  for (chunk = accumSpans->chunks; chunk; chunk = next) 
    {
      next = chunk->next;
      free (chunk);
    }
  accumSpans->chunks = (struct finalSpanChunk *)NULL;
  accumSpans->freeFinalSpans = (struct finalSpan *)NULL;
  free (accumSpans->finalSpans);
  accumSpans->finalSpans = (struct finalSpan **)NULL;
}


/**********************************************************************/
/* A sub-module, used by drawArc(), that generates the spans associated
   with an arc, and writes them to an in-core span accumulation by calling
   newFinalSpan().  When this is used, computeAcc() and computeBounds()
   have already been called, to compute `accelerators' (frequently used
   quantities associated with the ellipse).  hookX() and tailX() are called
   to do additional geometry computations. */
/**********************************************************************/

/* forward references */
static void arcSpan ____P((miAccumSpans *accumSpans, int y, int lx, int lw, int rx, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, unsigned int mask));
static void arcSpan0 ____P((miAccumSpans *accumSpans, int lx, int lw, int rx, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, unsigned int mask));
static void tailSpan ____P((miAccumSpans *accumSpans, int y, int lw, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, unsigned int mask));

/* Generate the set of spans with the given y coordinate. */
static void
#ifdef _HAVE_PROTOS
arcSpan (miAccumSpans *accumSpans, int y, int lx, int lw, int rx, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, unsigned int mask)
#else
arcSpan (accumSpans, y, lx, lw, rx, rw, def, bounds, acc, mask)
     miAccumSpans *accumSpans;
     int y, lx, lw, rx, rw;
     const struct arc_def *def;
     const struct arc_bound *bounds;
     const struct accelerators *acc;
     unsigned int mask;
#endif
{
  int linx, loutx, rinx, routx;
  double x, altx;

  if (boundedLe (y, bounds->inneri)) 
    {
      linx = -(lx + lw);
      rinx = rx;
    } 
  else 
    {
      /*
       * intersection with left face
       */
      x = hookX (y + acc->fromIntY, def, bounds, acc, true);
      if (acc->right.valid
	  && boundedLe (y + acc->fromIntY, bounds->right))
	{
	  altx = intersectLine (y + acc->fromIntY, acc->right);
	  if (altx < x)
	    x = altx;
	}
      linx = -ICEIL(acc->fromIntX - x);
      rinx = ICEIL(acc->fromIntX + x);
    }

  if (boundedLe (y, bounds->outeri)) 
    {
      loutx = -lx;
      routx = rx + rw;
    } 
  else 
    {
      /*
       * intersection with right face
       */
      x = hookX (y + acc->fromIntY, def, bounds, acc, false);
      if (acc->left.valid
	  && boundedLe (y + acc->fromIntY, bounds->left))
	{
	  altx = x;
	  x = intersectLine (y + acc->fromIntY, acc->left);
	  if (x < altx)
	    x = altx;
	}
      loutx = -ICEIL(acc->fromIntX - x);
      routx = ICEIL(acc->fromIntX + x);
    }

  if (routx > rinx) 
    {
      if (mask & 1)
	newFinalSpan (accumSpans, 
		      acc->yorgu - y,
		      acc->xorg + rinx, acc->xorg + routx);
      if (mask & 8)
	newFinalSpan (accumSpans, 
		      acc->yorgl + y,
		      acc->xorg + rinx, acc->xorg + routx);
    }

  if (loutx > linx) 
    {
      if (mask & 2)
	newFinalSpan (accumSpans, 
		      acc->yorgu - y,
		      acc->xorg - loutx, acc->xorg - linx);
      if (mask & 4)
	newFinalSpan (accumSpans, 
		      acc->yorgl + y,
		      acc->xorg - loutx, acc->xorg - linx);
    }
}

static void
#ifdef _HAVE_PROTOS
arcSpan0 (miAccumSpans *accumSpans, int lx, int lw, int rx, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, unsigned int mask)
#else
arcSpan0 (accumSpans, lx, lw, rx, rw, def, bounds, acc, mask)
     miAccumSpans *accumSpans;
     int lx, lw, rx, rw;
     const struct arc_def *def;
     const struct arc_bound *bounds;
     const struct accelerators *acc;
     unsigned int mask;
#endif
{
  double x;

  if (boundedLe (0, bounds->inneri) 
      && acc->left.valid && boundedLe (0, bounds->left)
      && acc->left.b > 0)
    {
      x = def->w - def->l;
      if (acc->left.b < x)
	x = acc->left.b;
      lw = ICEIL(acc->fromIntX - x) - lx;
      rw += rx;
      rx = ICEIL(acc->fromIntX + x);
      rw -= rx;
    }
  arcSpan (accumSpans, 0, lx, lw, rx, rw, def, bounds, acc, mask);
}

static void
#ifdef _HAVE_PROTOS
tailSpan (miAccumSpans *accumSpans, int y, int lw, int rw, const struct arc_def *def, const struct arc_bound *bounds, const struct accelerators *acc, unsigned int mask)
#else
tailSpan (accumSpans, y, lw, rw, def, bounds, acc, mask)
     miAccumSpans *accumSpans;
     int y, lw, rw;
     const struct arc_def *def;
     const struct arc_bound *bounds;
     const struct accelerators *acc;
     unsigned int mask;
#endif
{
  double yy, xalt, x, lx, rx;
  int n;

  if (boundedLe(y, bounds->outeri))
    arcSpan (accumSpans, y, 0, lw, -rw, rw, def, bounds, acc, mask);
  else if (def->w != def->h) 
    {
      yy = y + acc->fromIntY;
      x = tailX(yy, def, bounds, acc);
      if (yy == 0.0 && x == -rw - acc->fromIntX)
	return;
      if (acc->right.valid && boundedLe (yy, bounds->right)) 
	{
	  rx = x;
	  lx = -x;
	  xalt = intersectLine (yy, acc->right);
	  if (xalt >= -rw - acc->fromIntX && xalt <= rx)
	    rx = xalt;
	  n = ICEIL(acc->fromIntX + lx);
	  if (lw > n) 
	    {
	      if (mask & 2)
		newFinalSpan (accumSpans, 
			      acc->yorgu - y,
			      acc->xorg + n, acc->xorg + lw);
	      if (mask & 4)
		newFinalSpan (accumSpans, 
			      acc->yorgl + y,
			      acc->xorg + n, acc->xorg + lw);
	    }
	  n = ICEIL(acc->fromIntX + rx);
	  if (n > -rw) 
	    {
	      if (mask & 1)
		newFinalSpan (accumSpans, 
			      acc->yorgu - y,
			      acc->xorg - rw, acc->xorg + n);
	      if (mask & 8)
		newFinalSpan (accumSpans, 
			      acc->yorgl + y,
			      acc->xorg - rw, acc->xorg + n);
	    }
	}
      arcSpan (accumSpans, y,
	       ICEIL(acc->fromIntX - x), 0,
	       ICEIL(acc->fromIntX + x), 0,
	       def, bounds, acc, mask);
    }
}


/**********************************************************************/
/* The drawArc() function, which draws an arc to an in-core span
   accumulation by invoking the functions in the previous sub-module.  This
   calls miComputeWideEllipse() to rasterize the ellipse of which the arc
   is a part, and the helper functions computeAcc() and computeBounds().
   It is the low-level `draw to memory' function invoked by miArcSegment().

   drawZeroArc(), which follows, is simpler; it draws a degenerate
   (horizontal or vertical) arc. */
/**********************************************************************/

/* forward references */
static void drawQuadrant ____P((miAccumSpans *accumSpans, struct arc_def *def, struct accelerators *acc, int a0, int a1, unsigned int mask, miArcFace *right, miArcFace *left, miArcSpanData *spdata));
static void mirrorSppPoint ____P((int quadrant, SppPoint *sppPoint));

/* Split an arc into pieces which are scan-converted in the first quadrant
 * and mirrored into position.  This is necessary as the scan-conversion
 * code can only deal with arcs completely contained in the first quadrant.
 */
static void
#ifdef _HAVE_PROTOS
drawArc (miAccumSpans *accumSpans, const miArc *tarc, unsigned int l, int a0, int a1, miArcFace *right, miArcFace *left, miEllipseCache *ellipseCache)
#else
drawArc (accumSpans, tarc, l, a0, a1, right, left, ellipseCache)
     miAccumSpans *accumSpans;
     const miArc *tarc;
     unsigned int l;
     int a0, a1;
     miArcFace *right, *left;	/* these save arc endpoints */
     miEllipseCache *ellipseCache;
#endif
{
  struct arc_def	def;
  struct accelerators	acc;
  int			startq, endq, curq;
  int			rightq, leftq = 0, righta = 0, lefta = 0;
  miArcFace		*passRight, *passLeft;
  int			q0 = 0, q1 = 0;
  unsigned int		mask;
  struct band 
    {
      int		a0, a1;
      unsigned int	mask;
    }			band[5], sweep[20];
  int			bandno, sweepno;
  int			i, j;
  bool			flipRight = false, flipLeft = false;
  bool			copyEnd = false;
  miArcSpanData		*spdata;
  bool			mustFree;

  /* compute span data for the whole wide ellipse, also caching it for
     speedy later retrieval */
  spdata = miComputeWideEllipse (l, tarc, &mustFree, ellipseCache);
  if (!spdata)
    /* unknown failure, so punt */
    return;

  if (a1 < a0)
    a1 += 360 * 64;
  startq = a0 / (90 * 64);
  if (a0 == a1)
    endq = startq;
  else
    endq = (a1-1) / (90 * 64);
  bandno = 0;
  curq = startq;
  rightq = -1;
  for (;;) 
    {
      switch (curq) 
	{
	case 0:
	  if (a0 > 90 * 64)
	    q0 = 0;
	  else
	    q0 = a0;
	  if (a1 < 360 * 64)
	    q1 = IMIN (a1, 90 * 64);
	  else
	    q1 = 90 * 64;
	  if (curq == startq && a0 == q0 && rightq < 0) 
	    {
	      righta = q0;
	      rightq = curq;
	    }
	  if (curq == endq && a1 == q1) 
	    {
	      lefta = q1;
	      leftq = curq;
	    }
	  break;
	case 1:
	  if (a1 < 90 * 64)
	    q0 = 0;
	  else
	    q0 = 180 * 64 - IMIN (a1, 180 * 64);
	  if (a0 > 180 * 64)
	    q1 = 90 * 64;
	  else
	    q1 = 180 * 64 - IMAX (a0, 90 * 64);
	  if (curq == startq && 180 * 64 - a0 == q1) 
	    {
	      righta = q1;
	      rightq = curq;
	    }
	  if (curq == endq && 180 * 64 - a1 == q0) 
	    {
	      lefta = q0;
	      leftq = curq;
	    }
	  break;
	case 2:
	  if (a0 > 270 * 64)
	    q0 = 0;
	  else
	    q0 = IMAX (a0, 180 * 64) - 180 * 64;
	  if (a1 < 180 * 64)
	    q1 = 90 * 64;
	  else
	    q1 = IMIN (a1, 270 * 64) - 180 * 64;
	  if (curq == startq && a0 - 180*64 == q0) 
	    {
	      righta = q0;
	      rightq = curq;
	    }
	  if (curq == endq && a1 - 180 * 64 == q1) 
	    {
	      lefta = q1;
	      leftq = curq;
	    }
	  break;
	case 3:
	  if (a1 < 270 * 64)
	    q0 = 0;
	  else
	    q0 = 360 * 64 - IMIN (a1, 360 * 64);
	  q1 = 360 * 64 - IMAX (a0, 270 * 64);
	  if (curq == startq && 360 * 64 - a0 == q1) 
	    {
	      righta = q1;
	      rightq = curq;
	    }
	  if (curq == endq && 360 * 64 - a1 == q0) 
	    {
	      lefta = q0;
	      leftq = curq;
	    }
	  break;
	}
      band[bandno].a0 = q0;
      band[bandno].a1 = q1;
      band[bandno].mask = 1 << curq;
      bandno++;
      if (curq == endq)
	break;
      curq++;
      if (curq == 4) 
	{
	  a0 = 0;
	  a1 -= 360 * 64;
	  curq = 0;
	  endq -= 4;
	}
    }
  sweepno = 0;
  for (;;) 
    {
      q0 = 90 * 64;
      mask = 0;
      /*
       * find left-most point
       */
      for (i = 0; i < bandno; i++)
	if (band[i].a0 <= q0) 
	  {
	    q0 = band[i].a0;
	    q1 = band[i].a1;
	    mask = band[i].mask;
	  }
      if (mask == 0)
	break;
      /*
       * locate next point of change
       */
      for (i = 0; i < bandno; i++)
	if (!(mask & band[i].mask)) 
	  {
	    if (band[i].a0 == q0) 
	      {
		if (band[i].a1 < q1)
		  q1 = band[i].a1;
		mask |= band[i].mask;
	      } 
	    else if (band[i].a0 < q1)
	      q1 = band[i].a0;
	  }
      /*
       * create a new sweep
       */
      sweep[sweepno].a0 = q0;
      sweep[sweepno].a1 = q1;
      sweep[sweepno].mask = mask;
      sweepno++;
      /*
       * subtract the sweep from the affected bands
       */
      for (i = 0; i < bandno; i++)
	if (band[i].a0 == q0) 
	  {
	    band[i].a0 = q1;
	    /*
	     * check if this band is empty
	     */
	    if (band[i].a0 == band[i].a1)
	      band[i].a1 = band[i].a0 = 90 * 64 + 1;
	  }
    }
  computeAcc (tarc, l, &def, &acc);
  for (j = 0; j < sweepno; j++) 
    {
      mask = sweep[j].mask;
      passRight = passLeft = (miArcFace *)NULL;
      if (mask & (1 << rightq)) 
	{
	  if (sweep[j].a0 == righta)
	    passRight = right;
	  else if (sweep[j].a1 == righta) 
	    {
	      passLeft = right;
	      flipRight = true;
	    }
	}
      if (mask & (1 << leftq)) 
	{
	  if (sweep[j].a1 == lefta)
	    {
	      if (passLeft)
		copyEnd = true;
	      passLeft = left;
	    }
	  else if (sweep[j].a0 == lefta) 
	    {
	      if (passRight)
		copyEnd = true;
	      passRight = left;
	      flipLeft = true;
	    }
	}

      drawQuadrant (accumSpans, &def, &acc, sweep[j].a0, sweep[j].a1, mask, 
		    passRight, passLeft, spdata);
    }

  /* when copyEnd is true, both ends of the arc were computed at the same
   * time; drawQuadrant only takes one end though, so the left end will be
   * the only one holding the data.  Copy it from there.
   */
  if (copyEnd)
    *right = *left;
  /*
   * mirror the coordinates generated for the
   * faces of the arc
   */
  if (right) 
    {
      mirrorSppPoint (rightq, &right->clock);
      mirrorSppPoint (rightq, &right->center);
      mirrorSppPoint (rightq, &right->counterClock);
      if (flipRight) 
	{
	  SppPoint	temp;

	  temp = right->clock;
	  right->clock = right->counterClock;
	  right->counterClock = temp;
	}
    }
  if (left) 
    {
      mirrorSppPoint (leftq,  &left->counterClock);
      mirrorSppPoint (leftq,  &left->center);
      mirrorSppPoint (leftq,  &left->clock);
      if (flipLeft) 
	{
	  SppPoint	temp;

	  temp = left->clock;
	  left->clock = left->counterClock;
	  left->counterClock = temp;
	}
    }
  if (mustFree)
    {
      free (spdata->spans);
      free (spdata);
    }
}

static void
#ifdef _HAVE_PROTOS
drawQuadrant (miAccumSpans *accumSpans, struct arc_def *def, struct accelerators *acc, int a0, int a1, unsigned int mask, miArcFace *right, miArcFace *left, miArcSpanData *spdata)
#else
drawQuadrant (accumSpans, def, acc, a0, a1, mask, right, left, spdata)
     miAccumSpans *accumSpans;
     struct arc_def *def;
     struct accelerators *acc;
     int a0, a1;
     unsigned int mask;
     miArcFace *right, *left;
     miArcSpanData *spdata;	/* rasterized wide ellipse */
#endif
{
  struct arc_bound	bound;
  double		yy, x, xalt;
  int			y, miny, maxy;
  int			n;
  miArcSpan		*span;

  def->a0 = ((double) a0) / 64.0;
  def->a1 = ((double) a1) / 64.0;
  computeBound (def, &bound, acc, right, left);

  yy = bound.inner.min;
  if (bound.outer.min < yy)
    yy = bound.outer.min;
  miny = ICEIL(yy - acc->fromIntY);
  yy = bound.inner.max;
  if (bound.outer.max > yy)
    yy = bound.outer.max;
  maxy = (int)floor(yy - acc->fromIntY);
  y = spdata->k;
  span = spdata->spans;

  if (spdata->top)
    /* rasterized ellipse contains a `top point' */
    {
      if (a1 == 90 * 64 && (mask & 1))
	newFinalSpan (accumSpans, 
		      acc->yorgu - y - 1, acc->xorg, acc->xorg + 1);
      span++;
    }

  /* loop through one-span ArcSpans, at successive values of y */
  for (n = spdata->count1; --n >= 0; )
    {
      if (y < miny)
	return;
      if (y <= maxy) 
	{
	  /* generate spans at this y value */
	  arcSpan (accumSpans, y,
		   span->lx, -span->lx, 0, span->lx + span->lw,
		   def, &bound, acc, mask);
	  if (span->rw + span->rx)
	    tailSpan (accumSpans, y, -span->rw, -span->rx, def, &bound, acc, mask);
	}
      y--;
      span++;
    }
  if (y < miny)
    return;

  if (spdata->hole)
    /* have a one-pixel hole to fill in */
    {
      if (y <= maxy)
	/* generate a one-point span at this y value */
	arcSpan (accumSpans, y, 0, 0, 0, 1, 
		 def, &bound, acc, mask & 0xc);
    }

  /* loop through two-span ArcSpans, at successive values of y */
  for (n = spdata->count2; --n >= 0; )
    {
      if (y < miny)
	return;
      if (y <= maxy)
	/* generate the two spans at this y value */
	arcSpan (accumSpans, y, span->lx, span->lw, span->rx, span->rw,
		 def, &bound, acc, mask);
      
      y--;
      span++;
    }

  if (spdata->bot && miny <= y && y <= maxy)
    /* have a `horizontal centerline' ArcSpan; treat it specially */
    {
      unsigned int m = mask;

      if (y == miny)
	m &= 0xc;
      if (span->rw <= 0) 
	{
	  arcSpan0 (accumSpans, span->lx, -span->lx, 0, span->lx + span->lw,
		    def, &bound, acc, m);
	  if (span->rw + span->rx)
	    tailSpan (accumSpans, y, -span->rw, -span->rx, def, &bound, acc, m);
	}
      else
	arcSpan0 (accumSpans, span->lx, span->lw, span->rx, span->rw,
		  def, &bound, acc, m);
      y--;
    }

  while (y >= miny) 
    {
      yy = y + acc->fromIntY;
      if (def->w == def->h) 
	{
	  xalt = def->w - def->l;
	  x = -sqrt(xalt * xalt - yy * yy);
	} 
      else 
	{
	  x = tailX(yy, def, &bound, acc);
	  if (acc->left.valid && boundedLe (yy, bound.left)) 
	    {
	      xalt = intersectLine (yy, acc->left);
	      if (xalt < x)
		x = xalt;
	    }
	  if (acc->right.valid && boundedLe (yy, bound.right)) 
	    {
	      xalt = intersectLine (yy, acc->right);
	      if (xalt < x)
		x = xalt;
	    }
	}
      /* generate span at this y value */
      arcSpan (accumSpans, y,
	       ICEIL(acc->fromIntX - x), 0,
	       ICEIL(acc->fromIntX + x), 0,
	       def, &bound, acc, mask);
      y--;
    }
}

static void
#ifdef _HAVE_PROTOS
mirrorSppPoint (int quadrant, SppPoint *sppPoint)
#else
mirrorSppPoint (quadrant, sppPoint)
     int quadrant;
     SppPoint *sppPoint;
#endif
{
  switch (quadrant) 
    {
    case 0:
      break;
    case 1:
      sppPoint->x = -sppPoint->x;
      break;
    case 2:
      sppPoint->x = -sppPoint->x;
      sppPoint->y = -sppPoint->y;
      break;
    case 3:
      sppPoint->y = -sppPoint->y;
      break;
    }
  /*
   * and translate to X coordinate system
   */
  sppPoint->y = -sppPoint->y;
}


/***********************************************************************/
/* Draw a degenerate (zero width/height) arc.  Left and right faces are
 * computed.  Called by miArcSegment() to handle the degenerate case:
 * tarc->width = 0 or tarc->height = 0. */
/***********************************************************************/

static void
#ifdef _HAVE_PROTOS
drawZeroArc (miAccumSpans *accumSpans, const miArc *tarc, unsigned int lw, miArcFace *left, miArcFace *right)
#else
drawZeroArc (accumSpans, tarc, lw, left, right)
     miAccumSpans *accumSpans;
     const miArc *tarc;
     unsigned int lw;
     miArcFace *left, *right;	/* these save arc endpoints */
#endif
{
  double	x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0;
  double	w, h, x, y;
  double	xmax, ymax, xmin, ymin;
  int		a0, a1;
  double	a, startAngle, endAngle;
  double	l, lx, ly;

  l = 0.5 * lw;
  a0 = tarc->angle1;
  a1 = tarc->angle2;
  if (a1 > FULLCIRCLE)
    a1 = FULLCIRCLE;
  else if (a1 < -FULLCIRCLE)
    a1 = -FULLCIRCLE;
  w = 0.5 * tarc->width;
  h = 0.5 * tarc->height;
  /*
   * play in X coordinates right away
   */
  startAngle = - ((double) a0 / 64.0);
  endAngle = - ((double) (a0 + a1) / 64.0);
	
  xmax = -w;
  xmin = w;
  ymax = -h;
  ymin = h;
  a = startAngle;
  for (;;)
    {
      x = w * miDcos(a);
      y = h * miDsin(a);
      if (a == startAngle)
	{
	  x0 = x;
	  y0 = y;
	}
      if (a == endAngle)
	{
	  x1 = x;
	  y1 = y;
	}
      if (x > xmax)
	xmax = x;
      if (x < xmin)
	xmin = x;
      if (y > ymax)
	ymax = y;
      if (y < ymin)
	ymin = y;
      if (a == endAngle)
	break;
      if (a1 < 0)		/* clockwise */
	{
	  if (floor (a / 90.0) == floor (endAngle / 90.0))
	    a = endAngle;
	  else
	    a = 90 * (floor (a/90.0) + 1);
	}
      else
	{
	  if (ceil (a / 90.0) == ceil (endAngle / 90.0))
	    a = endAngle;
	  else
	    a = 90 * (ceil (a/90.0) - 1);
	}
    }
  lx = ly = l;
  if ((x1 - x0) + (y1 - y0) < 0)
    lx = ly = -l;
  if (h)
    ly = 0.0;
  else
    lx = 0.0;
  if (right)
    {
      right->center.x = x0;
      right->center.y = y0;
      right->clock.x = x0 - lx;
      right->clock.y = y0 - ly;
      right->counterClock.x = x0 + lx;
      right->counterClock.y = y0 + ly;
    }
  if (left)
    {
      left->center.x = x1;
      left->center.y = y1;
      left->clock.x = x1 + lx;
      left->clock.y = y1 + ly;
      left->counterClock.x = x1 - lx;
      left->counterClock.y = y1 - ly;
    }
	
  x0 = xmin;
  x1 = xmax;
  y0 = ymin;
  y1 = ymax;
  if (ymin != y1) 
    {
      xmin = -l;
      xmax = l;
    } 
  else 
    {
      ymin = -l;
      ymax = l;
    }

  if (xmax != xmin && ymax != ymin) 
    /* construct a rectangle and `paint' it */
    {
      int		minx, maxx, miny, maxy;
      int		xorg, yorg, width, height;
      
      minx = ICEIL(xmin + w) + tarc->x;
      maxx = ICEIL(xmax + w) + tarc->x;
      miny = ICEIL(ymin + h) + tarc->y;
      maxy = ICEIL(ymax + h) + tarc->y;
      xorg = minx;
      yorg = miny;
      width = maxx - minx;
      height = maxy - miny;

      /* paint rectangle to the in-core miAccumSpans struct, except for its
         right and bottom edges */
      while (height--)
	newFinalSpan (accumSpans, yorg, xorg, xorg + width);
    }
}
