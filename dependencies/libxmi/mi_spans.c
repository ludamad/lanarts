/* This module provides several public functions: miNewPaintedSet(),
   miAddSpansToPaintedSet(), miUniquifyPaintedSet(), miClearPaintedSet(),
   miDeletePaintedSet().  They maintain a structure called a miPaintedSet,
   which is essentially an array of SpanGroup structures, one per pixel
   value.  A SpanGroup is essentially an unsorted list of Spans's.  A Spans
   is a list of spans (i.e. horizontal ranges) of miPoints, sorted so that
   the starting points have increasing y-values.  See mi_spans.h.

   Internally, each libxmi drawing function paints to a miPaintedSet by
   calling miAddSpansToPaintedSet() on one or more Spans's.  This function
   adds a Spans to a miPaintedSet, being careful first to remove each from
   the miPaintedSet each pixel in the Spans, if it has previously been
   painted another color.  However, for efficiency it does not check
   whether a pixel in the Spans has been previously painted the same color.
   So while the drawing function is being called, the Spans in any one of
   the PaintedSet's SpanGroups may overlap.  But different SpanGroups do
   not overlap.  That is an invariant.

   After all calls to miAddSpansToPaintedSet() are completed, duplicate
   pixels are resolved by invoking miUniquifyPaintedSet().  That takes
   place in the API wrappers in mi_api.c, just before the drawing function
   returns.

   The function miCopyPaintedSetToCanvas(), in mi_canvas.c, can copy the
   contents of a miPaintedSet, i.e. its spans of painted miPoints, to a
   miCanvas structure.  Sophisticated pixel merging is supported.  It would
   be easy to write other functions that copy pixels out of a
   miPaintedSet. */

/* Original version written by Joel McCormack, Summer 1989.  
   Hacked by Robert S. Maier, 1998-1999. */

#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_api.h"

/* spans in a Spans are sorted by y, so these give ymin, ymax for a
   nonempty Spans */
#define YMIN(spans) (spans->points[0].y)
#define YMAX(spans) (spans->points[spans->count-1].y)

/* internal functions */
static SpanGroup * miNewSpanGroup ____P((miPixel pixel));
static int miUniquifySpansX ____P((const Spans *spans, miPoint *newPoints, unsigned int *newWidths));
static void miAddSpansToSpanGroup ____P((const Spans *spans, SpanGroup *spanGroup));
static void miDeleteSpanGroup ____P((SpanGroup *spanGroup));
static void miQuickSortSpansX ____P((miPoint *points, unsigned int *widths, int numSpans));
static void miSubtractSpans ____P((SpanGroup *spanGroup, const Spans *sub));
static void miUniquifySpanGroup ____P((SpanGroup *spanGroup));



/* The following functions are the public functions of this module. */

miPaintedSet *
#ifdef _HAVE_PROTOS
miNewPaintedSet (void)
#else
miNewPaintedSet ()
#endif
{
  miPaintedSet *paintedSet;

  paintedSet = (miPaintedSet *)mi_xmalloc (sizeof(miPaintedSet));
  paintedSet->groups = (SpanGroup **)NULL; /* pointer-to-SpanGroup slots */
  paintedSet->size = 0;		/* slots allocated */
  paintedSet->ngroups = 0;	/* slots filled */

  return paintedSet;
}

/* Add a Spans to a miPaintedSet's SpanGroup for a specified pixel values,
   and also subtract it from the SpanGroups for all other pixel values. */
void
#ifdef _HAVE_PROTOS
miAddSpansToPaintedSet (const Spans *spans, miPaintedSet *paintedSet, miPixel pixel)
#else
miAddSpansToPaintedSet (spans, paintedSet, pixel)
     const Spans *spans;
     miPaintedSet *paintedSet;
     miPixel pixel;
#endif
{
  bool found = false;
  int i;
  SpanGroup *spanGroup;

  if (spans->count == 0)
    return;

  for (i = 0; i < paintedSet->ngroups; i++)
    {
      miPixel stored_pixel;
      
      stored_pixel = paintedSet->groups[i]->pixel;
      if (MI_SAME_PIXEL(pixel, stored_pixel))
	{
	  found = true;		/* have a spanGroup for this pixel value */
	  break;
	}
    }
  if (!found)
    {
      if (paintedSet->ngroups == paintedSet->size)
	/* expand array of SpanGroups */
	{
	  int old_size = paintedSet->size;
	  int new_size = 2 * (old_size + 8);
	  
	  if (old_size == 0)
	    paintedSet->groups = (SpanGroup **)
	      mi_xmalloc(new_size * sizeof(SpanGroup *));
	  else
	    paintedSet->groups = (SpanGroup **)
	      mi_xrealloc(paintedSet->groups, new_size * sizeof(SpanGroup *));
	  paintedSet->size = new_size;
	}

      /* create a SpanGroup for this pixel value */
      i = paintedSet->ngroups;
      paintedSet->groups[i] = miNewSpanGroup (pixel);
      paintedSet->ngroups++;
    }
  
  spanGroup = paintedSet->groups[i];
  miAddSpansToSpanGroup (spans, spanGroup);

  /* subtract Spans from all other SpanGroups */
  for (i = 0; i < paintedSet->ngroups; i++)
    {
      SpanGroup *otherGroup;
      
      otherGroup = paintedSet->groups[i];
      if (otherGroup == spanGroup)
	continue;
      miSubtractSpans (otherGroup, spans);
    }
}

/* Deallocate all of a miPaintedSet's SpanGroups, including the points and
   width arrays that are part of its component Spans's.  So it will
   effectively become the empty set, as if it had been newly created. */
void
#ifdef _HAVE_PROTOS
miClearPaintedSet (miPaintedSet *paintedSet)
#else
miClearPaintedSet (paintedSet)
     miPaintedSet *paintedSet;
#endif
{
  int i;

  if (paintedSet == (miPaintedSet *)NULL)
    return;

  for (i = 0; i < paintedSet->ngroups; i++)
    miDeleteSpanGroup (paintedSet->groups[i]);
  if (paintedSet->size > 0)
    free (paintedSet->groups);
  paintedSet->size = 0;		/* slots allocated */
  paintedSet->ngroups = 0;	/* slots filled */
}

/* Deallocate a miPaintedSet, including the points and width arrays that
   are part of its component Spans's. */
void
#ifdef _HAVE_PROTOS
miDeletePaintedSet (miPaintedSet *paintedSet)
#else
miDeletePaintedSet (paintedSet)
     miPaintedSet *paintedSet;
#endif
{
  int i;

  if (paintedSet == (miPaintedSet *)NULL)
    return;

  for (i = 0; i < paintedSet->ngroups; i++)
    miDeleteSpanGroup (paintedSet->groups[i]);

  if (paintedSet->size > 0)
    free (paintedSet->groups);
  free (paintedSet);
}

/* `Uniquify' a miPaintedSet, i.e. uniquify each of its SpanGroups (see
   below). */
void
#ifdef _HAVE_PROTOS
miUniquifyPaintedSet (miPaintedSet *paintedSet)
#else
miUniquifyPaintedSet (paintedSet)
     miPaintedSet *paintedSet;
#endif
{
  int i;

  if (paintedSet == (miPaintedSet *)NULL)
    return;

  for (i = 0; i < paintedSet->ngroups; i++)
    {
      if (paintedSet->groups[i]->count > 0)
	{
	  miUniquifySpanGroup (paintedSet->groups[i]);
	}
    }
}


/* Create and initialize a SpanGroup, i.e. an unsorted list of Spans's. */
static SpanGroup *
#ifdef _HAVE_PROTOS
miNewSpanGroup (miPixel pixel)
#else
miNewSpanGroup (pixel)
     miPixel pixel;
#endif
{
  SpanGroup *spanGroup;

  spanGroup = (SpanGroup *)mi_xmalloc (sizeof(SpanGroup));
  spanGroup->pixel = pixel;	/* pixel to be used */
  spanGroup->size = 0;		/* slots allocated */
  spanGroup->count = 0;		/* slots filled */
  spanGroup->group = (Spans *)NULL; /* slots for Spans's */
  spanGroup->ymin = INT_MAX;	/* min over slots */
  spanGroup->ymax = INT_MIN;	/* max over slots */

  return spanGroup;
}

/* Add a Spans to a SpanGroup, by tacking it on the end; update SpanGroup's
   ymin, ymax. */
static void
#ifdef _HAVE_PROTOS
miAddSpansToSpanGroup (const Spans *spans, SpanGroup *spanGroup)
#else
miAddSpansToSpanGroup (spans, spanGroup)
     const Spans *spans;
     SpanGroup *spanGroup;
#endif
{
  int ymin, ymax;

  if (spans->count == 0)
    return;
  if (spanGroup->size == spanGroup->count) 
    /* expand SpanGroup */
    {
      spanGroup->size = (spanGroup->size + 8) * 2;
      spanGroup->group = (Spans *)
	mi_xrealloc(spanGroup->group, sizeof(Spans) * spanGroup->size);
    }
  
  /* tack Spans onto end of SpanGroup, update SpanGroup's ymin and ymax */
  spanGroup->group[spanGroup->count] = *spans;
  (spanGroup->count)++;
  ymin = YMIN(spans);
  if (ymin < spanGroup->ymin) 
    spanGroup->ymin = ymin;
  ymax = YMAX(spans);
  if (ymax > spanGroup->ymax) 
    spanGroup->ymax = ymax;
}

/* Delete a SpanGroup, including the point and width arrays that are part
   of each Spans. */
static void
#ifdef _HAVE_PROTOS
miDeleteSpanGroup (SpanGroup *spanGroup)
#else
miDeleteSpanGroup (spanGroup)
     SpanGroup *spanGroup;
#endif
{
  int i;

  if (spanGroup == (SpanGroup *)NULL)
    return;

  for (i = 0; i < spanGroup->count; i++)
    {
      /*  free spanGroup->group[i], which is a Spans */
      free (spanGroup->group[i].points);
      free (spanGroup->group[i].widths);
    }
  if (spanGroup->group)
    free (spanGroup->group);
  free (spanGroup);
}

/* Subtract a Spans from a SpanGroup, i.e. from each of its Spans's; update
   SpanGroup's ymin, ymax. */
static void 
#ifdef _HAVE_PROTOS
miSubtractSpans (SpanGroup *spanGroup, const Spans *sub)
#else
miSubtractSpans (spanGroup, sub)
     SpanGroup *spanGroup;
     const Spans *sub;
#endif
{
  int		i, subCount, spansCount;
  int		ymin, ymax, xmin, xmax;
  Spans		*spans;
  miPoint	*subPt, *spansPt;
  unsigned int	*subWid, *spansWid;
  int		extra;
  bool		gross_change = false;

  if (sub->count == 0)		/* nothing to do */
    return;

  /* y range of Spans to be subtracted */
  ymin = YMIN(sub);
  ymax = YMAX(sub);

  /* loop through all Spans's in SpanGroup */
  spans = spanGroup->group;
  for (i = spanGroup->count; i > 0; i--, spans++) 
    {
      if (spans->count == 0)
	continue;

      /* look only at Spans's with y ranges that overlap with `sub' */
      if (YMIN(spans) <= ymax && ymin <= YMAX(spans)) 
	{
	  /* count, start points, and widths for `sub' */
	  subCount = sub->count;
	  subPt = sub->points;
	  subWid = sub->widths;

	  /* count, start points, and widths for current Spans */
	  spansCount = spans->count;
	  spansPt = spans->points;
	  spansWid = spans->widths;

	  extra = 0;		/* extra span slots available in Spans */
	  for (;;)
	    /* look at pairs of spans, one from each Spans, that have the
	       same value for y (break out when one or the other Spans is
	       exhausted) */
	    {
	      while (spansCount && spansPt->y < subPt->y)
		{
		  spansPt++;  
		  spansWid++; 
		  spansCount--;
		}
	      if (!spansCount)
		break;
	      while (subCount && subPt->y < spansPt->y)
		{
		  subPt++;
		  subWid++;
		  subCount--;
		}
	      if (!subCount)
		break;

	      if (subPt->y == spansPt->y)
		/* the two spans are at same y value, analyse in detail */
		{
		  xmin = subPt->x;
		  xmax = xmin + (int)(*subWid); /* just right of sub span */
		  if (xmin >= spansPt->x + (int)(*spansWid)
		      || spansPt->x >= xmax)
		    /* non-overlapping, do nothing */
		    {
		      ;
		    }
		  else if (xmin <= spansPt->x)
		    /* span to be subtracted begins at the same point, or
                       to the left */
		    {
		      if (xmax >= spansPt->x + (int)(*spansWid))
			/* span to be subtracted ends at the same point,
			   or to the right; delete this span by downshifting */
			{
			  memmove (spansPt, spansPt + 1, 
				   sizeof(miPoint) * (spansCount - 1));
			  memmove (spansWid, spansWid + 1, 
				   sizeof(unsigned int) * (spansCount - 1));
			  spansPt--;
			  spansWid--;
			  spans->count--;
			  extra++;
			  gross_change = true; /* span vanished */
			}
		      else 
			/* span to be subtracted ends to the left of this
			   one's ending point; alter ending point and width */
			{
			  *spansWid = 
			    *spansWid - (unsigned int)(xmax - spansPt->x);
			  spansPt->x = xmax;
			}
		    }
		  else
		    /* span to be subtracted overlaps with this one, and
                        begins to the right of this one */
		    {
		      if (xmax >= spansPt->x + (int)*spansWid)
			/* span to be subtracted ends at the same point, or
			   to the right; just update width */
			*spansWid = (unsigned int)(xmin - spansPt->x);
		      else
			/* hard case: must split the span */
			{
#define EXTRA 8
			  if (extra == 0)
			    /* reallocate; create EXTRA new span slots */
			    {
			      miPoint *newPt;
			      unsigned int *newwid;

			      newPt = (miPoint *)mi_xrealloc (spans->points, 
				       (spans->count + EXTRA)*sizeof(miPoint));
			      spansPt = newPt + (spansPt - spans->points);
			      spans->points = newPt;
			      newwid = (unsigned int *)mi_xrealloc (spans->widths, 
			          (spans->count + EXTRA)*sizeof(unsigned int));
			      spansWid = newwid + (spansWid - spans->widths);
			      spans->widths = newwid;
			      extra = EXTRA;
			    }

			  /* downshift; create two new spans as replacement */
			  memmove (spansPt + 1, spansPt, 
				   sizeof(miPoint) * spansCount);
			  memmove (spansWid + 1, spansWid, 
				   sizeof(unsigned int) * spansCount);
			  spans->count++;
			  extra--;
			  /* first new span */
			  *spansWid = (unsigned int)(xmin - spansPt->x);
			  spansWid++;
			  spansPt++;
			  /* second new span */
			  *spansWid = *spansWid - (unsigned int)(xmax - spansPt->x);
			  spansPt->x = xmax;
			}
		    }
		} /* end of same-value-of-y computations */

	      /* on to next span in the Spans */
	      spansPt++;  
	      spansWid++; 
	      spansCount--;
	    }
	}
    }

  if (gross_change)
    /* at least one span vanished; recompute SpanGroup's ymin, ymax */
    {
      ymax = INT_MIN;
      ymin = INT_MAX;

      /* loop through all Spans's in SpanGroup */
      spans = spanGroup->group;
      for (i = spanGroup->count; i > 0; i--, spans++) 
	{
	  int ymin_spans, ymax_spans;

	  if (spans->count == 0)
	    continue;
	  ymin_spans = YMIN(spans);
	  ymax_spans = YMAX(spans);
	  if (ymin_spans < ymin)
	    ymin = ymin_spans;
	  if (ymax_spans > ymax)
	    ymax = ymax_spans;
	}

      spanGroup->ymin = ymin;
      spanGroup->ymax = ymax;      
    }
}

/* `Uniquify' a SpanGroup: merge all its Spans's into a single Spans, which
   will be sorted on x as well as on y. */
static void
#ifdef _HAVE_PROTOS
miUniquifySpanGroup (SpanGroup *spanGroup)
#else
miUniquifySpanGroup (spanGroup)
     SpanGroup *spanGroup;
#endif
{
  int    i;
  Spans  *spans;
  Spans  *yspans;
  int    *ysizes;
  int    ymin, ylength;

  /* the new single Spans */
  miPoint *points;
  unsigned int *widths;
  int count;

  if (spanGroup->count == 0) 
    return;

  /* Special case : ymin > ymax, so the Spans's in the SpanGroup, no matter
     how numerous, must be empty (and can't contain point or width arrays).  */
  if (spanGroup->ymin > spanGroup->ymax)
    {
      spanGroup->count = 0;
      return;
    }

  /* Yuck.  Gross.  Radix sort into y buckets, then sort x and uniquify */
  /* This seems to be the fastest thing to do.  I've tried sorting on
     both x and y at the same time rather than creating into all those
     y buckets, but it was somewhat slower. */
  
  ymin    = spanGroup->ymin;
  ylength = spanGroup->ymax - ymin + 1;
  
  /* allocate Spans's for y buckets (one Spans for every scanline);
     ysizes[] is number of allocated Span slots in each bucket */
  yspans = (Spans *)mi_xmalloc (ylength * sizeof(Spans));
  ysizes = (int *)mi_xmalloc (ylength * sizeof(int));
  for (i = 0; i < ylength; i++) 
    {
      ysizes[i]        = 0;
      yspans[i].count  = 0;
      yspans[i].points = (miPoint *)NULL;
      yspans[i].widths = (unsigned int *)NULL;
    }
  
  /* go through every single span and put it into the correct y bucket */
  count = 0;
  for (i = 0, spans = spanGroup->group; 
       i < spanGroup->count; i++, spans++) 
    {
      int j, index;
      
      for (j = 0, points = spans->points, widths = spans->widths;
	   j < spans->count; j++, points++, widths++) 
	{
	  index = points->y - ymin;
	  if (index >= 0 && index < ylength) /* paranoia */
	    {
	      Spans *newspans = &(yspans[index]);
	      
	      if (newspans->count == ysizes[index]) 
		/* expand bucket arrays by reallocating */
		{
		  ysizes[index] = (ysizes[index] + 8) * 2;
		  newspans->points
		    = (miPoint *)mi_xrealloc (newspans->points,
					      ysizes[index] * sizeof(miPoint));
		  newspans->widths
		    = (unsigned int *)mi_xrealloc (newspans->widths,
						   ysizes[index] * sizeof(unsigned int));
		}
	      newspans->points[newspans->count] = *points;
	      newspans->widths[newspans->count] = *widths;
	      (newspans->count)++;
	    } /* if y value of span in range */
	} /* for j through spans */
      
      count += spans->count;
    } /* for i through Spans */
  free (ysizes);
  
  /* now sort each bucket by x and uniquify it into new Spans */
  points = (miPoint *)mi_xmalloc (count * sizeof(miPoint));
  widths = (unsigned int *)mi_xmalloc (count * sizeof(unsigned int));
  count = 0;
  for (i = 0; i < ylength; i++) 
    {
      int ycount = yspans[i].count;
      
      if (ycount > 0) 
	{
	  if (ycount > 1) 
	    /* sort the >1 spans at this value of y */
	    {
	      miQuickSortSpansX (yspans[i].points, yspans[i].widths, ycount);
	      count += miUniquifySpansX
		(&(yspans[i]), &(points[count]), &(widths[count]));
	    } 
	  else 
	    /* just a single span at this value of y */
	    {
	      points[count] = yspans[i].points[0];
	      widths[count] = yspans[i].widths[0];
	      count++;
	    }
	  free (yspans[i].points);
	  free (yspans[i].widths);
	}
    }
  free (yspans);
  
  /* free SpanGroup's original Spans's, including Span arrays */
  for (i = 0; i < spanGroup->count; i++)
    {
      free (spanGroup->group[i].points);
      free (spanGroup->group[i].widths);
    }

  /* SpanGroup now has only a single Spans */
  spanGroup->count = 1;
  spanGroup->group[0].points = points;
  spanGroup->group[0].widths = widths;
  spanGroup->group[0].count = count;
}


/* Sort each span in a Spans by x.  Called only if numSpans > 1. */
static void 
#ifdef _HAVE_PROTOS
miQuickSortSpansX (miPoint *points, unsigned int *widths, int numSpans)
#else
miQuickSortSpansX (points, widths, numSpans)
     miPoint *points;
     unsigned int *widths;
     int numSpans;
#endif
{
  int	 x;
  int	 i, j, m;
  miPoint *r;

#define ExchangeSpans(a, b)				    \
  {							    \
    miPoint tpt;	     					    \
    unsigned int tw;					    \
								\
    tpt = points[a]; points[a] = points[b]; points[b] = tpt;    \
    tw = widths[a]; widths[a] = widths[b]; widths[b] = tw;  \
  }

  do 
    {
      if (numSpans < 9) 
	/* do insertion sort */
	{
	  int xprev;

	  xprev = points[0].x;
	  i = 1;
	  do 			/* while i != numSpans */
	    {
	      x = points[i].x;
	      if (xprev > x) 
		{
		  /* points[i] is out of order.  Move into proper location. */
		  miPoint tpt;
		  unsigned int tw;
		  int k;

		  for (j = 0; x >= points[j].x; j++) 
		    { 
		    }
		  tpt = points[i];
		  tw  = widths[i];
		  for (k = i; k != j; k--) 
		    {
		      points[k] = points[k-1];
		      widths[k] = widths[k-1];
		    }
		  points[j] = tpt;
		  widths[j] = tw;
		  x = points[i].x;
		} /* if out of order */
	      xprev = x;
	      i++;
	    } while (i != numSpans);

	  /* end of insertion sort */
	  return;
	}

      /* Choose partition element, stick in location 0 */
      m = numSpans / 2;
      if (points[m].x > points[0].x)
	ExchangeSpans(m, 0);
      if (points[m].x > points[numSpans-1].x) 
	ExchangeSpans(m, numSpans-1);
      if (points[m].x > points[0].x)
	ExchangeSpans(m, 0);
      x = points[0].x;

      /* Partition array */
      i = 0;
      j = numSpans;
      do 
	{
	  r = &(points[i]);
	  do 
	    {
	      r++;
	      i++;
	    } 
	  while (i != numSpans && r->x < x)
	    ;
	  r = &(points[j]);
	  do 
	    {
	      r--;
	      j--;
	    } 
	  while (x < r->x);
	  if (i < j) ExchangeSpans(i, j);
	} 
      while (i < j);

      /* Move partition element back to middle */
      ExchangeSpans(0, j);

      /* Recurse */
      if (numSpans-j-1 > 1)
	miQuickSortSpansX (&points[j+1], &widths[j+1], numSpans-j-1);
      numSpans = j;
    } while (numSpans > 1);
}

/* Sort an unordered list of spans by y, so that it becomes a Spans. */
void 
#ifdef _HAVE_PROTOS
miQuickSortSpansY (miPoint *points, unsigned int *widths, int numSpans)
#else
miQuickSortSpansY (points, widths, numSpans)
     miPoint *points;
     unsigned int *widths;
     int numSpans;
#endif
{
  int	 y;
  int	 i, j, m;
  miPoint *r;

  if (numSpans <= 1)		/* nothing to do */
    return;

#define ExchangeSpans(a, b)				    \
  {							    \
    miPoint tpt;	     					    \
    unsigned int tw;					    \
								\
    tpt = points[a]; points[a] = points[b]; points[b] = tpt;    \
    tw = widths[a]; widths[a] = widths[b]; widths[b] = tw;  \
  }

  do 
    {
      if (numSpans < 9) 
	/* do insertion sort */
	{
	  int yprev;

	  yprev = points[0].y;
	  i = 1;
	  do 			/* while i != numSpans */
	    {
	      y = points[i].y;
	      if (yprev > y) 
		{
		  /* points[i] is out of order.  Move into proper location. */
		  miPoint tpt;
		  unsigned int tw;
		  int k;

		  for (j = 0; y >= points[j].y; j++) 
		    { 
		    }
		  tpt = points[i];
		  tw  = widths[i];
		  for (k = i; k != j; k--) 
		    {
		      points[k] = points[k-1];
		      widths[k] = widths[k-1];
		    }
		  points[j] = tpt;
		  widths[j] = tw;
		  y = points[i].y;
		} /* if out of order */
	      yprev = y;
	      i++;
	    } while (i != numSpans);

	  /* end of insertion sort */
	  return;
	}

      /* Choose partition element, stick in location 0 */
      m = numSpans / 2;
      if (points[m].y > points[0].y)
	ExchangeSpans(m, 0);
      if (points[m].y > points[numSpans-1].y) 
	ExchangeSpans(m, numSpans-1);
      if (points[m].y > points[0].y)
	ExchangeSpans(m, 0);
      y = points[0].y;

      /* Partition array */
      i = 0;
      j = numSpans;
      do 
	{
	  r = &(points[i]);
	  do 
	    {
	      r++;
	      i++;
	    } 
	  while (i != numSpans && r->y < y)
	    ;
	  r = &(points[j]);
	  do 
	    {
	      r--;
	      j--;
	    } 
	  while (y < r->y);
	  if (i < j) ExchangeSpans(i, j);
	} 
      while (i < j);

      /* Move partition element back to middle */
      ExchangeSpans(0, j);

      /* Recurse */
      if (numSpans-j-1 > 1)
	miQuickSortSpansY (&points[j+1], &widths[j+1], numSpans-j-1);
      numSpans = j;
    } while (numSpans > 1);
}

/* Uniquify the spans in a Spans.  (Spans at each y value are assumed to
   have been sorted on x, perhaps by calling miQuickSortSpansX() above.)
   Also, create a new Spans: stash the uniquified spans into the previously
   allocated arrays newPoints and newWidths.  Returns the number of unique
   spans.  Called only if numSpans > 1. */
static int 
#ifdef _HAVE_PROTOS
miUniquifySpansX (const Spans *spans, miPoint *newPoints, unsigned int *newWidths)
#else
miUniquifySpansX (spans, newPoints, newWidths)
     const Spans *spans;
     miPoint *newPoints;
     unsigned int *newWidths;
#endif
{
  int		newx1, newx2, oldpt, i, y;
  miPoint	*oldPoints;
  unsigned int	*oldWidths, *startNewWidths;

  startNewWidths = newWidths;
  oldPoints = spans->points;
  oldWidths = spans->widths;
  y = oldPoints->y;
  newx1 = oldPoints->x;
  newx2 = newx1 + (int)(*oldWidths);

  for (i = spans->count - 1; i > 0; i--) 
    {
      oldPoints++;
      oldWidths++;
      oldpt = oldPoints->x;
      if (oldpt > newx2) 
	{
	  /* write current span, start a new one */
	  newPoints->x = newx1;
	  newPoints->y = y;
	  *newWidths = (unsigned int)(newx2 - newx1);
	  newPoints++;
	  newWidths++;
	  newx1 = oldpt;
	  newx2 = oldpt + (int)(*oldWidths);
	} 
      else 
	{
	  /* extend current span, if old extends beyond new */
	  oldpt = oldpt + (int)(*oldWidths);
	  if (oldpt > newx2)
	    newx2 = oldpt;
	}
    }

  /* write final span */
  newPoints->x = newx1;
  *newWidths = (unsigned int)(newx2 - newx1);
  newPoints->y = y;

  return (int)((newWidths - startNewWidths) + 1);
}
