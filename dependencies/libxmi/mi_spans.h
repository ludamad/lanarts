/* This is the public include file for the miPaintedSet module, contained
   in mi_spans.c. */

/* A Spans structure is a sorted list of spans, i.e. a list of point ranges
   [xmin,xmax], sorted in increasing y order.  There may be more than one
   span at a given y. */

typedef struct 
{
  int		count;		/* number of spans		    */
  miPoint	*points;	/* pointer to list of start points  */
  unsigned int	*widths;	/* pointer to list of widths	    */
} Spans;

/* A SpanGroup is an unsorted list of Spans's, associated with a particular
   pixel value.  

   A SpanGroup is allowed to include more than a single Spans because most
   libxmi drawing functions write out multiple Spans's. */

typedef struct 
{
    miPixel	pixel;		/* pixel value				*/
    Spans       *group;		/* Spans slots				*/
    int		size;		/* number of Spans slots allocated	*/
    int		count;		/* number of Spans slots filled		*/
    int		ymin, ymax;	/* min, max y values over all Spans's	*/
} SpanGroup;

/* A miPaintedSet structure is an array of SpanGroups, specifying the
   partition into differently painted subsets.  There is at most one
   SpanGroup for any pixel. */

typedef struct lib_miPaintedSet
{
  SpanGroup	**groups;	/* SpanGroup slots			*/
  int		size;		/* number of SpanGroup slots allocated	*/
  int		ngroups;	/* number of SpanGroup slots filled	*/
} _miPaintedSet;

/* libxmi's low-level painting macro.  It `paints' a Spans, i.e. a list of
   spans assumed to be in y-increasing order, to a miPaintedSet with a
   specified pixel value.  To do this, it invokes the lower-level function
   miAddSpansToPaintedSet() in mi_spans.c.

   The passed point and width arrays should have been allocated on the
   heap, since they will be eventually freed; e.g., when the miPaintedSet
   is cleared or deallocated. */

#define MI_PAINT_SPANS(paintedSet, pixel, numSpans, ppts, pwidths) \
  {\
    Spans 	spanRec;\
    if (numSpans > 0) \
      { \
        spanRec.points = (ppts);\
        spanRec.widths = (pwidths);\
        spanRec.count = (numSpans);\
        miAddSpansToPaintedSet (&spanRec, (paintedSet), (pixel));\
      } \
    else \
      { \
        free (ppts); \
        free (pwidths); \
      } \
    }

/* A wrapper for MI_PAINT_SPANS() that can be applied to a span array
   (i.e. to a point and width array) that can't be freed, so must be
   copied.  We try not to use this. */
#define MI_COPY_AND_PAINT_SPANS(paintedSet, pixel, nPts, FirstPoint, FirstWidth) \
  {\
    if ((nPts) > 0) \
      { \
        miPoint		*ppt, *pptInit, *oldppt; \
        unsigned int	*pwidth, *pwidthInit, *oldpwidth; \
        int		ptsCounter; \
        ppt = pptInit = (miPoint *) mi_xmalloc ((nPts) * sizeof (miPoint));\
        pwidth = pwidthInit = (unsigned int *) mi_xmalloc ((nPts) * sizeof (unsigned int));\
        oldppt = FirstPoint;\
        oldpwidth = FirstWidth;\
        for (ptsCounter = (nPts); --ptsCounter >= 0; )\
          {\
            *ppt++ = *oldppt++;\
            *pwidth++ = *oldpwidth++;\
          }\
        MI_PAINT_SPANS(paintedSet, pixel, (nPts), pptInit, pwidthInit)\
      } \
  }

/* miPaintedSet manipulation routines (other than public) */
extern void miAddSpansToPaintedSet ____P((const Spans *spans, miPaintedSet *paintedSet, miPixel pixel));
extern void miQuickSortSpansY ____P((miPoint *points, unsigned int *widths, int numSpans));
extern void miUniquifyPaintedSet ____P((miPaintedSet *paintedSet));
