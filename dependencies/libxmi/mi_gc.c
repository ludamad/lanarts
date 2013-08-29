#include "sys-defines.h"
#include "extern.h"

/* These functions create, manipulate and destroy miGC structures.  A
   pointer to an miGC is passed as the third argument to each of libxmi's
   public drawing functions.  It comprises high-level drawing parameters.
   The miGC structure is defined in mi_gc.h. */

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"

/* create a new miGC, with elements initialized to default values (the same
   default values that are used by X11) */

miGC * 
#ifdef _HAVE_PROTOS
miNewGC (int npixels, const miPixel *pixels)
#else
miNewGC (npixels, pixels)
     int npixels;
     const miPixel *pixels;
#endif
{
  miGC *new_gc;
  int i;
  
  new_gc = (miGC *)mi_xmalloc (sizeof (miGC));
  new_gc->fillRule = MI_EVEN_ODD_RULE;
  new_gc->joinStyle = MI_JOIN_MITER;  
  new_gc->capStyle = MI_CAP_BUTT;  
  new_gc->lineStyle = MI_LINE_SOLID;  
  new_gc->arcMode = MI_ARC_PIE_SLICE;  
  new_gc->lineWidth = (unsigned int)0;
  new_gc->miterLimit = 10.43;	/* same as hardcoded in X11 */
  new_gc->dashOffset = 0;
  new_gc->numInDashList = 2;
  new_gc->dash = (unsigned int *)mi_xmalloc (2 * sizeof(unsigned int));
  for (i = 0; i < 2; i++)
    new_gc->dash[i] = 4;	/* { 4, 4 }; same as in X11? */
  new_gc->numPixels = npixels;
  new_gc->pixels = (miPixel *)mi_xmalloc (npixels * sizeof (miPixel));
  for (i = 0; i < npixels; i++)
    new_gc->pixels[i] = pixels[i];
  
  return new_gc;
}

/* destroy (deallocate) an miGC */

void
#ifdef _HAVE_PROTOS
miDeleteGC (miGC *pGC)
#else
miDeleteGC (pGC)
    miGC *pGC;
#endif
{
  if (pGC == (miGC *)NULL)
    return;
  if (pGC->dash)
    free (pGC->dash);
  free (pGC->pixels);
  free (pGC);
}

/* copy an miGC */

miGC * 
#ifdef _HAVE_PROTOS
miCopyGC (const miGC *pGC)
#else
miCopyGC (pGC)
     const miGC *pGC;
#endif
{
  miGC *new_gc;
  int i;
  
  if (pGC == (const miGC *)pGC)
    return (miGC *)NULL;

  new_gc = (miGC *)mi_xmalloc (sizeof (miGC));
  new_gc->fillRule = pGC->fillRule;
  new_gc->joinStyle = pGC->joinStyle;
  new_gc->capStyle = pGC->capStyle;
  new_gc->lineStyle = pGC->lineStyle;
  new_gc->arcMode = pGC->arcMode;
  new_gc->lineWidth = pGC->lineWidth;
  new_gc->miterLimit = pGC->miterLimit;
  new_gc->dashOffset = pGC->dashOffset;
  new_gc->numInDashList = pGC->numInDashList;
  if (pGC->numInDashList == 0)
    new_gc->dash = (unsigned int *)NULL;
  else
    {
      new_gc->dash = 
	(unsigned int *)mi_xmalloc (pGC->numInDashList * sizeof(unsigned int));
      for (i = 0; i < pGC->numInDashList; i++)
	new_gc->dash[i] = pGC->dash[i];
    }
  new_gc->pixels = 
    (miPixel *)mi_xmalloc (pGC->numPixels * sizeof(miPixel));
  for (i = 0; i < pGC->numPixels; i++)
    new_gc->pixels[i] = pGC->pixels[i];

  return new_gc;
}

/* set a single integer-valued miGC attribute */

void
#ifdef _HAVE_PROTOS
miSetGCAttrib (miGC *pGC, miGCAttribute attribute, int value)
#else
miSetGCAttrib (pGC, attribute, value)
     miGC *pGC;
     miGCAttribute attribute;
     int value;
#endif
{
  if (pGC == (miGC *)NULL || value < 0)
    return;
  switch ((int)attribute)
    {
    case (int)MI_GC_FILL_RULE:
      pGC->fillRule = value;
      break;
    case (int)MI_GC_JOIN_STYLE:
      pGC->joinStyle = value;
      break;
    case (int)MI_GC_CAP_STYLE:
      pGC->capStyle = value;
      break;
    case (int)MI_GC_LINE_STYLE:
      pGC->lineStyle = value;
      break;
    case (int)MI_GC_ARC_MODE:
      pGC->arcMode = value;
      break;
    case (int)MI_GC_LINE_WIDTH:
      if (value >= 0)
	pGC->lineWidth = (unsigned int)value;
      break;
    default:			/* unknown attribute type */
      break;
    }
}

/* set many integer-valued miGC attributes, at a single time */

void
#ifdef _HAVE_PROTOS
miSetGCAttribs (miGC *pGC, int nattributes, const miGCAttribute *attributes, const int *values)
#else
miSetGCAttribs (pGC, nattributes, attributes, values)
     miGC *pGC;
     int nattributes;
     const miGCAttribute *attributes;
     const int *values;
#endif
{
  int i;
  miGCAttribute attribute;
  int value;
  
  if (nattributes <= 0 || pGC == (miGC *)NULL)
    return;
  for (i = 0; i < nattributes; i++)
    {
      attribute = *attributes++;
      value = *values++;

      if (value < 0)		/* invalid; be tolerant */
	continue;
      switch ((int)attribute)
	{
	case (int)MI_GC_FILL_RULE:
	  pGC->fillRule = value;
	  break;
	case (int)MI_GC_JOIN_STYLE:
	  pGC->joinStyle = value;
	  break;
	case (int)MI_GC_CAP_STYLE:
	  pGC->capStyle = value;
	  break;
	case (int)MI_GC_LINE_STYLE:
	  pGC->lineStyle = value;
	  break;
	case (int)MI_GC_ARC_MODE:
	  pGC->arcMode = value;
	  break;
	case (int)MI_GC_LINE_WIDTH:
	  if (value >= 0)
	    pGC->lineWidth = (unsigned int)value;
	  break;
	default:			/* unknown attribute type */
	  break;
	}
    }
}

/* set the only float-valued miGC attribute (the miter limit) */

void
#ifdef _HAVE_PROTOS
miSetGCMiterLimit (miGC *pGC, double value)
#else
miSetGCMiterLimit (pGC, value)
     miGC *pGC;
     double value;
#endif
{
  if (pGC == (miGC *)NULL)
    return;
  pGC->miterLimit = value;
}

/* set the dash-related attributes in an miGC */

void
#ifdef _HAVE_PROTOS
miSetGCDashes (miGC *pGC, int ndashes, const unsigned int *dashes, int offset)
#else
miSetGCDashes (pGC, ndashes, dashes, offset)
     miGC *pGC;
     int ndashes;
     const unsigned int *dashes;
     int offset;
#endif
{
  int i;

  if (pGC == (miGC *)NULL || ndashes < 0)
    return;
  if (pGC->dash)
    free (pGC->dash);
  pGC->dashOffset = offset;
  pGC->numInDashList = ndashes;
  if (ndashes == 0)
    pGC->dash = (unsigned int *)NULL;
  else
    {
      pGC->dash = (unsigned int *)mi_xmalloc (ndashes * sizeof(unsigned int));
      for (i = 0; i < ndashes; i++)
	pGC->dash[i] = dashes[i];
    }
}

/* set the pixel array in a miGC */
void 
#ifdef _HAVE_PROTOS
miSetGCPixels (miGC *pGC, int npixels, const miPixel *pixels)
#else
miSetGCPixels (pGC, npixels, pixels)
     miGC *pGC;
     int npixels;
     const miPixel *pixels;
#endif
{
  int i;

  if (pGC == (miGC *)NULL || npixels < 2)
    return;
  free (pGC->pixels);
  pGC->numPixels = npixels;
  pGC->pixels = (miPixel *)mi_xmalloc (npixels * sizeof (miPixel));
  for (i = 0; i < npixels; i++)
    pGC->pixels[i] = pixels[i];
}
