/* This file defines libxmi's miCanvas class.  An miCanvas is an object
   that includes a drawable (a miCanvasPixmap, which by default is a
   miPixmap [essentially a 2-D array of miPixels]) and various data members
   that indicate how any specified set of pixels should be painted.  These
   may include a stipple miBitmap and a texture miPixmap; if they are
   non-NULL, they will tile the canvas.

   This file supports a good deal of customization, which would probably be
   needed if libxmi is installed as a rendering module in other software.
   The following preprocessor symbols may be defined:

   MI_PIXEL_TYPE
   MI_CANVAS_DRAWABLE_TYPE
   MI_GET_CANVAS_DRAWABLE_PIXEL()
   MI_SET_CANVAS_DRAWABLE_PIXEL()
   MI_GET_CANVAS_DRAWABLE_BOUNDS()
   MI_DEFAULT_MERGE2_PIXEL()
   MI_DEFAULT_MERGE3_PIXEL()

   See xmi.h for details on what they do.  Ideally you should be able to
   #define those symbols appropriately, and never modify this file. */

#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_api.h"

/* forward references (these are currently used only in this file) */
static miPixmap * miNewPixmap ____P((unsigned int width, unsigned int height, miPixel initPixel));
static miPixmap * miCopyPixmap ____P((const miPixmap *pPixmap));
static void miDeletePixmap ____P((miPixmap *pPixmap));
#if 0		/* not currently used, so commented out */
static miBitmap * miNewBitmap ____P((unsigned int width, unsigned int height, int initBit));
#endif
static miBitmap * miCopyBitmap ____P((const miBitmap *pBitmap));
static void miDeleteBitmap ____P((miBitmap *pBitmap));
static void miPaintCanvas ___P((miCanvas *canvas, miPixel pixel, int n, const miPoint *ppt, const unsigned int *pwidth, miPoint offset));

/* Ctor/dtor/copy ctor for the miCanvas class.  These are defined only if
   the symbol MI_CANVAS_DRAWABLE_TYPE hasn't been defined by the libxmi
   installer.  If it's defined, the drawable encapsulated within an
   miCanvas is presumably something other than a miPixmap, and we won't
   know how to create/destroy/copy miCanvas's. */

#ifndef MI_CANVAS_DRAWABLE_TYPE

/* create (allocate) a new miCanvas */
miCanvas * 
#ifdef _HAVE_PROTOS
miNewCanvas (unsigned int width, unsigned int height, miPixel initPixel)
#else
miNewCanvas (width, height, initPixel)
     unsigned int width, height;
     miPixel initPixel;
#endif
{
  miCanvas *new_pCanvas;
  
  if (width < 1 || height < 1)
    return (miCanvas *)NULL;

  new_pCanvas = (miCanvas *)mi_xmalloc (sizeof (miCanvas));
  new_pCanvas->drawable = miNewPixmap (width, height, initPixel);

  /* default values */
  new_pCanvas->texture = (miPixmap *)NULL;
  new_pCanvas->stipple = (miBitmap *)NULL;
  new_pCanvas->pixelMerge2 = (miPixelMerge2)NULL;
  new_pCanvas->pixelMerge3 = (miPixelMerge3)NULL;

  return new_pCanvas;
}

/* copy a miCanvas */
miCanvas * 
#ifdef _HAVE_PROTOS
miCopyCanvas (const miCanvas *pCanvas)
#else
miCopyCanvas (pCanvas)
     const miCanvas *pCanvas;
#endif
{
  miCanvas *new_pCanvas;
  
  if (pCanvas == (const miCanvas *)NULL)
    return (miCanvas *)NULL;

  new_pCanvas = (miCanvas *)mi_xmalloc (sizeof (miCanvas));
  new_pCanvas->drawable = miCopyPixmap (pCanvas->drawable);
  new_pCanvas->pixelMerge2 = pCanvas->pixelMerge2;
  new_pCanvas->pixelMerge3 = pCanvas->pixelMerge3;
  new_pCanvas->texture = miCopyPixmap (pCanvas->texture);
  new_pCanvas->stipple = miCopyBitmap (pCanvas->stipple);

  return new_pCanvas;
}

/* destroy (deallocate) an miCanvas */
void
#ifdef _HAVE_PROTOS
miDeleteCanvas (miCanvas *pCanvas)
#else
miDeleteCanvas (pCanvas)
    miCanvas *pCanvas;
#endif
{
  if (pCanvas == (miCanvas *)NULL)
    return;

  miDeletePixmap (pCanvas->drawable);
  miDeletePixmap (pCanvas->texture);
  miDeleteBitmap (pCanvas->stipple);
  free (pCanvas);
}

#endif /* not MI_CANVAS_DRAWABLE_TYPE */

/* create a new miPixmap, and fill it with a specified miPixel */
static miPixmap * 
#ifdef _HAVE_PROTOS
miNewPixmap (unsigned int width, unsigned int height, miPixel initPixel)
#else
miNewPixmap (width, height, initPixel)
     unsigned int width, height;
     miPixel initPixel;
#endif
{
  miPixmap *new_pPixmap;
  miPixel **pixmap;
  int i, j;
  
  new_pPixmap = (miPixmap *)mi_xmalloc (sizeof(miPixmap));

  /* create a pixmap (an array of pointers to rows of miPixels) */
  pixmap = (miPixel **)mi_xmalloc (height * sizeof(miPixel *));
  for (j = 0; j < (int)height; j++)
    {
      pixmap[j] = (miPixel *)mi_xmalloc (width * sizeof(miPixel));
      for (i = 0; i < (int)width; i++)
	pixmap[j][i] = initPixel;
    }

  new_pPixmap->pixmap = pixmap;
  new_pPixmap->width = width;
  new_pPixmap->height = height;

  return new_pPixmap;
}

/* copy a miPixmap */
static miPixmap * 
#ifdef _HAVE_PROTOS
miCopyPixmap (const miPixmap *pPixmap)
#else
miCopyPixmap (pPixmap)
     const miPixmap *pPixmap;
#endif
{
  miPixmap *new_pPixmap;
  miPixel **pixmap;
  miPixel * const *old_pixmap;
  int i, j;
  
  if (pPixmap == (const miPixmap *)NULL)
    return (miPixmap *)NULL;

  new_pPixmap = (miPixmap *)mi_xmalloc (sizeof(miPixmap));

  /* create a pixmap (an array of pointers to rows of miPixels) */
  pixmap = (miPixel **)mi_xmalloc (pPixmap->height * sizeof(miPixel *));
  old_pixmap = pPixmap->pixmap;
  for (j = 0; j < (int)(pPixmap->height); j++)
    {
      pixmap[j] = (miPixel *)mi_xmalloc (pPixmap->width * sizeof(miPixel));
      for (i = 0; i < (int)(pPixmap->width); i++)
	pixmap[j][i] = old_pixmap[j][i];
    }

  new_pPixmap->pixmap = pixmap;
  new_pPixmap->width = pPixmap->width;
  new_pPixmap->height = pPixmap->height;

  return new_pPixmap;
}

/* destroy (deallocate) an miPixmap */
static void
#ifdef _HAVE_PROTOS
miDeletePixmap (miPixmap *pPixmap)
#else
miDeletePixmap (pPixmap)
    miPixmap *pPixmap;
#endif
{
  int j;

  if (pPixmap == (miPixmap *)NULL)
    return;

  /* free pixmap (an array of pointers to rows of miPixels) */
  for (j = 0; j < (int)(pPixmap->height); j++)
    free (pPixmap->pixmap[j]);
  free (pPixmap->pixmap);

  free (pPixmap);
}

#if 0		/* not currently used, so commented out */
/* create a new miBitmap, and fill it with a specified value (only 0 and 1
   are meaningful) */
static miBitmap * 
#ifdef _HAVE_PROTOS
miNewBitmap (unsigned int width, unsigned int height, int initBit)
#else
miNewBitmap (width, height, initBit)
     unsigned int width, height;
     int initBit;
#endif
{
  miBitmap *new_pBitmap;
  int **bitmap;
  int i, j;
  
  new_pBitmap = (miBitmap *)mi_xmalloc (sizeof(miBitmap));

  /* create a bitmap (an array of pointers to rows of ints) */
  bitmap = (int **)mi_xmalloc (height * sizeof(int *));
  for (j = 0; j < (int)height; j++)
    {
      bitmap[j] = (int *)mi_xmalloc (width * sizeof(int));
      for (i = 0; i < (int)width; i++)
	bitmap[j][i] = initBit;
    }

  new_pBitmap->bitmap = bitmap;
  new_pBitmap->width = width;
  new_pBitmap->height = height;

  return new_pBitmap;
}
#endif

/* copy a miBitmap */
static miBitmap * 
#ifdef _HAVE_PROTOS
miCopyBitmap (const miBitmap *pBitmap)
#else
miCopyBitmap (pBitmap)
     const miBitmap *pBitmap;
#endif
{
  miBitmap *new_pBitmap;
  int **bitmap;
  int * const *old_bitmap;
  int i, j;
  
  if (pBitmap == (const miBitmap *)NULL)
    return (miBitmap *)NULL;

  new_pBitmap = (miBitmap *)mi_xmalloc (sizeof(miBitmap));

  /* create a bitmap (an array of pointers to rows of ints) */
  bitmap = (int **)mi_xmalloc (pBitmap->height * sizeof(int *));
  old_bitmap = pBitmap->bitmap;
  for (j = 0; j < (int)(pBitmap->height); j++)
    {
      bitmap[j] = (int *)mi_xmalloc (pBitmap->width * sizeof(int));
      for (i = 0; i < (int)(pBitmap->width); i++)
	bitmap[j][i] = old_bitmap[j][i];
    }

  new_pBitmap->bitmap = bitmap;
  new_pBitmap->width = pBitmap->width;
  new_pBitmap->height = pBitmap->height;

  return new_pBitmap;
}

/* destroy (deallocate) an miBitmap */
static void
#ifdef _HAVE_PROTOS
miDeleteBitmap (miBitmap *pBitmap)
#else
miDeleteBitmap (pBitmap)
    miBitmap *pBitmap;
#endif
{
  int j;

  if (pBitmap == (miBitmap *)NULL)
    return;

  /* free bitmap (an array of pointers to rows of ints) */
  for (j = 0; j < (int)(pBitmap->height); j++)
    free (pBitmap->bitmap[j]);
  free (pBitmap->bitmap);

  free (pBitmap);
}

/* set the binary pixel-merging function in an miCanvas */
void 
#ifdef _HAVE_PROTOS
miSetPixelMerge2 (miCanvas *pCanvas, miPixelMerge2 pixelMerge2)
#else
miSetPixelMerge2 (pCanvas, pixelMerge2)
     miCanvas *pCanvas;
     miPixelMerge2 pixelMerge2;
#endif
{
  if (pCanvas == (miCanvas *)NULL)
    return;
  pCanvas->pixelMerge2 = pixelMerge2;
}

/* set the ternary pixel-merging function in an miCanvas */
void 
#ifdef _HAVE_PROTOS
miSetPixelMerge3 (miCanvas *pCanvas, miPixelMerge3 pixelMerge3)
#else
miSetPixelMerge3 (pCanvas, pixelMerge3)
     miCanvas *pCanvas;
     miPixelMerge3 pixelMerge3;
#endif
{
  if (pCanvas == (miCanvas *)NULL)
    return;
  pCanvas->pixelMerge3 = pixelMerge3;
}

/* Copy a stipple miBitmap into an miCanvas.  The old stipple, if any, is
   deallocated. */
void 
#ifdef _HAVE_PROTOS
miSetCanvasStipple (miCanvas *pCanvas, const miBitmap *pstipple, miPoint stippleOrigin)
#else
miSetCanvasStipple (pCanvas, pstipple, stippleOrigin)
     miCanvas *pCanvas;
     const miBitmap *pstipple;
     miPoint stippleOrigin;
#endif
{
  if (pCanvas == (miCanvas *)NULL)
    return;

  miDeleteBitmap (pCanvas->stipple);
  pCanvas->stipple = miCopyBitmap (pstipple);
  pCanvas->stippleOrigin = stippleOrigin;
}

/* Copy a texture miPixmap into an miCanvas.  The old texture, if any, is
   deallocated. */
void 
#ifdef _HAVE_PROTOS
miSetCanvasTexture (miCanvas *pCanvas, const miPixmap *pTexture, miPoint textureOrigin)
#else
miSetCanvasTexture (pCanvas, pTexture, textureOrigin)
     miCanvas *pCanvas;
     const miPixmap *pTexture;
     miPoint textureOrigin;
#endif
{
  if (pCanvas == (miCanvas *)NULL)
    return;

  miDeletePixmap (pCanvas->texture);
  pCanvas->texture = miCopyPixmap (pTexture);
  pCanvas->textureOrigin = textureOrigin;
}

/* Paint a list of spans, in a specified miPixel color, to a canvas.  The
   spans must be in y-increasing order. */
static void 
#ifdef _HAVE_PROTOS
miPaintCanvas (miCanvas *canvas, miPixel pixel, int n, const miPoint *ppt, const unsigned int *pwidth, miPoint offset)
#else
miPaintCanvas (canvas, pixel, n, ppt, pwidth, offset)
     miCanvas *canvas;		/* canvas */
     miPixel pixel;		/* source pixel color */
     int n;			/* number of spans to be painted */
     const miPoint *ppt;	/* array of starting points of spans */
     const unsigned int *pwidth; /* array of widths of spans */
     miPoint offset;		/* point that (0,0) gets mapped to */
#endif
{
  int i;
  int xleft, xright, ybottom, ytop;
  unsigned int stippleWidth = 0, stippleHeight = 0; /* keep lint happy */
  unsigned int textureWidth = 0, textureHeight = 0; /* keep lint happy */
  int stippleXOrigin = 0, stippleYOrigin = 0; /* keep lint happy */
  int textureXOrigin = 0, textureYOrigin = 0;	/* keep lint happy */
  int xstart, xend, xstart_clip, xend_clip, xoffset, yoffset, x, y;
  unsigned int width;
  const miCanvas *pCanvas;	/* `const' should be OK here */
  miPixelMerge2 pixelMerge2;
  miPixelMerge3 pixelMerge3;

  pCanvas = canvas;
  xoffset = offset.x;
  yoffset = offset.y;

  /* compute bounds of destination drawable */
  MI_GET_CANVAS_DRAWABLE_BOUNDS(pCanvas, xleft, ytop, xright, ybottom)

  /* if source doesn't overlap with destination drawable, do nothing */
  if (ppt[0].y + yoffset > ybottom || ppt[n-1].y + yoffset < ytop)
    return;

  /* determine user-specified merging functions (if any) */
  pixelMerge2 = pCanvas->pixelMerge2;
  pixelMerge3 = pCanvas->pixelMerge3;

#define MI_MERGE_CANVAS_PIXEL(pCanvas, x, y, sourcePixel, texturePixel, have_texturePixel) \
{ \
  miPixel destinationPixel, newPixel; \
  MI_GET_CANVAS_DRAWABLE_PIXEL((pCanvas), (x), (y), destinationPixel); \
  if (!have_texturePixel) \
    { \
      if (pixelMerge2 != (miPixelMerge2)NULL) \
        newPixel = (*pixelMerge2)((sourcePixel), destinationPixel); \
      else \
        MI_DEFAULT_MERGE2_PIXEL(newPixel, (sourcePixel), destinationPixel); \
    } \
  else \
    { \
      if (pixelMerge3 != (miPixelMerge3)NULL) \
        newPixel = (*pixelMerge3)((texturePixel), (sourcePixel), destinationPixel); \
      else \
        MI_DEFAULT_MERGE3_PIXEL(newPixel, (texturePixel), (sourcePixel), destinationPixel); \
    } \
  MI_SET_CANVAS_DRAWABLE_PIXEL((pCanvas), (x), (y), newPixel); \
}

  if (pCanvas->stipple)
    {
      stippleWidth = pCanvas->stipple->width;
      stippleHeight = pCanvas->stipple->height;  
      stippleXOrigin = pCanvas->stippleOrigin.x;
      stippleYOrigin = pCanvas->stippleOrigin.y;
      while (stippleXOrigin > 0)
	stippleXOrigin -= stippleWidth;
      while (stippleYOrigin > 0)
	stippleYOrigin -= stippleHeight;
    }
  
  if (pCanvas->texture)
    {
      textureWidth = pCanvas->texture->width;
      textureHeight = pCanvas->texture->height;  
      textureXOrigin = pCanvas->textureOrigin.x;
      textureYOrigin = pCanvas->textureOrigin.y;
      while (textureXOrigin > 0)
	textureXOrigin -= textureWidth;
      while (textureYOrigin > 0)
	textureYOrigin -= textureHeight;
    }

  for (i = 0; i < n; i++)
    {
      y = ppt[i].y + yoffset;
      if (y > ybottom)
	return;			/* no more spans will be painted */
      if (y >= ytop)
	{
	  width = pwidth[i];
	  xstart = ppt[i].x + xoffset;
	  xend = xstart + (int)width - 1;
	  
	  xstart_clip = IMAX(xstart,xleft);
	  xend_clip = IMIN(xend,xright);
	  
	  for (x = xstart_clip; x <= xend_clip; x++) /* may be empty */
	    /* merge pixel onto canvas */
	    {
	      miPixel texturePixel, sourcePixel;
	      bool have_texturePixel = false;
	      
	      if (pCanvas->texture)
		{
		  texturePixel = pCanvas->texture->pixmap[(y-textureYOrigin) % textureHeight][(x-textureXOrigin) % textureWidth];
		  have_texturePixel = true;
		}
	      else
		texturePixel = pixel; /* dummy; keep lint happy */

	      sourcePixel = pixel;
	      
	      if (pCanvas->stipple == (miBitmap *)NULL
		  || pCanvas->stipple->bitmap[(y-stippleYOrigin) % stippleHeight][(x-stippleXOrigin) % stippleWidth] != 0)
		MI_MERGE_CANVAS_PIXEL(pCanvas, x, y, sourcePixel, texturePixel, have_texturePixel)
	    } /* end for x in ... */
	} /* end if */
    } /* end for y in ... */
}

/* Copy a miPaintedSet to an miCanvas.  The miPaintedSet is assumed to have
   been uniquified (see mi_spans.c), which is the case after any of the
   eight core drawing functions in the libxmi API has been invoked.  So
   there is at most one Spans per SpanGroup (i.e., Spans #0), and copying
   pixels out of it trivial. */
void
#ifdef _HAVE_PROTOS
miCopyPaintedSetToCanvas (const miPaintedSet *paintedSet, miCanvas *canvas, miPoint offset)
#else
miCopyPaintedSetToCanvas (paintedSet, canvas, offset)
     const miPaintedSet *paintedSet;
     miCanvas *canvas;
     miPoint offset;		/* point that (0,0) is mapped to */
#endif
{
  int i;

  /* For each pixel color, the initial Spans in the corresponding SpanGroup
     is effectively a list of spans, in y-increasing order.  That is, it's
     a list of starting points and a corresponding list of widths; both are
     of length `count'. */

  for (i = 0; i < paintedSet->ngroups; i++)
    if (paintedSet->groups[i]->group[0].count > 0)
      miPaintCanvas (canvas, paintedSet->groups[i]->pixel,
		     paintedSet->groups[i]->group[0].count, 
		     paintedSet->groups[i]->group[0].points, 
		     paintedSet->groups[i]->group[0].widths, offset);
}
