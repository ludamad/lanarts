/* This is xmi.h, the public header file for the machine-independent libxmi
   rasterization library, which is based on source files in the X Window
   System distribution.  Those files are copyright (c) 1985-1989 by the X
   Consortium.  For the original authors, and an X Consortium permission
   notice, see the accompanying file README-X.  GNU extensions by 
   Robert S. Maier <rsm@math.arizona.edu> copyright (c) 1998-2000 by the
   Free Software Foundation.

   The eight painting functions in libxmi's core API, namely 

     the 5 drawing functions:
       miDrawPoints, miDrawLines, miDrawRectangles, miDrawArcs, miDrawArcs_r

     the 3 filling functions:     
       miFillPolygon, miFillRectangles, miFillArcs  

   are declared below.  The first argument of each of these is a pointer to
   a miPaintedSet.  Conceptually, a miPaintedSet is a set of points with
   integer coordinates, each of which is painted some color (a miPixel).
   The coordinates of the points are unconstrained, i.e., the core painting
   functions perform no clipping.

   Each of the core painting functions takes as second argument a pointer
   to a graphics context: a miGC.  A miGC is an opaque type that contains
   high-level drawing parameters that determine which points will be added
   to the miPaintedSet.  (For example, the line width, line style, and dash
   pattern, all of which are relevant to the drawing functions, though not
   to the filling functions.)  It also specifies the colors (miPixels) that
   will be used when painting the points.  The core painting functions use
   the Painter's Algorithm, so that if a point in an miPaintedSet is
   painted a second time, the new color will replace the old.

   (By default, the painting performed by the core painting functions,
   i.e. by both the drawing functions and the filling functions, is
   `solid', i.e., non-interpolated.  In `solid' painting, the color used is
   taken from the pixel array in the miGC.  Any miGC contains an array of
   pixel colors, of length n>=2.  Color #1 is the default color for
   painting, and colors 0,2,3,..,n-1 are used only by the drawing
   functions, when drawing in a dashed mode.  In normal (on/off) dashing,
   the colors of the `on' dashes will cycle through 1,2,..,n-1.  In
   so-called double dashing, the `off' dashes will be drawn too, in color #0.)

   After a miPaintedSet is built up by invoking one or more core painting
   functions, the next stage of the graphics pipeline is performed by
   calling miCopyPaintedSetToCanvas().  This transfers the pixels in the
   miPaintedSet onto a canvas structure called a miCanvas, which contains a
   bounded, fixed-size drawable.  In the transfer, more sophisticated
   algorithms than the Painter's Algorithm may be used.  Besides the
   drawable, a miCanvas may contain additional members, such as a stipple
   bitmap, a texture pixmap, and binary and ternary pixel-merging
   functions.  These will affect how the pixels from the miPaintedSet are
   combined with the ones that already exist on the drawable. */

/* This file is written for ANSI C compilers.  If you use it with a
   pre-ANSI C compiler that does not support the `const' keyword, such as
   the `cc' compiler supplied with SunOS (i.e., Solaris 1.x), you should
   use the -DNO_CONST_SUPPORT option when compiling your code. */

#ifndef _XMI_H_
#define _XMI_H_ 1

/***********************************************************************/

/* Version of GNU libxmi which this header file accompanies.  This
   information is included beginning with version 1.2.

   The MI_LIBXMI_VER_STRING macro is compiled into the library, as
   `mi_libxmi_ver'.  The MI_LIBXMI_VER macro is not compiled into it.  Both
   are available to applications that include this header file. */

#define MI_LIBXMI_VER_STRING "1.2"
#define MI_LIBXMI_VER         120

extern const char mi_libxmi_ver[8]; /* need room for 99.99aa */

/**********************************************************************/

/* support C++ */
#ifdef ___BEGIN_DECLS
#undef ___BEGIN_DECLS
#endif
#ifdef ___END_DECLS
#undef ___END_DECLS
#endif
#ifdef __cplusplus
# define ___BEGIN_DECLS extern "C" {
# define ___END_DECLS }
#else
# define ___BEGIN_DECLS		/* empty */
# define ___END_DECLS		/* empty */
#endif
     
/* ___P is a macro used to wrap function prototypes, so that compilers that
   don't understand ANSI C prototypes still work, and ANSI C compilers can
   issue warnings about type mismatches. */
#ifdef ___P
#undef ___P
#endif
#if defined (__STDC__) || defined (_AIX) \
	|| (defined (__mips) && defined (_SYSTYPE_SVR4)) \
	|| defined(WIN32) || defined(__cplusplus)
#define ___P(protos) protos
#else
#define ___P(protos) ()
#endif

/* Support old compilers with no `const' support (e.g. SunOS cc). */
#ifdef ___const
#undef ___const
#endif
#ifdef NO_CONST_SUPPORT
#ifdef const
#undef const
#endif /* const */
#define const
#define ___const
#else
#define ___const const
#endif

/* Support truly ancient compilers with no `void' support. */
#ifdef voidptr_t
#undef voidptr_t
#endif /* voidptr_t */
#ifdef NO_VOID_SUPPORT
#ifdef void
#undef void
#endif /* void */
#define voidptr_t char *
#define void int
#else  /* not NO_VOID_SUPPORT */
#define voidptr_t void *
#endif /* not NO_VOID_SUPPORT */

/**********************************************************************/

/* Structure that defines a point with integer coordinates. */
typedef struct
{
  int x, y;			/* integer coordinates, y goes downward */
} miPoint;

/* Definition of miPixel, the pixel value datatype.  By default, a miPixel
   is an unsigned int.  The libxmi installer may alter the definition by
   defining the symbol MI_PIXEL_TYPE at installation time.  The macro
   MI_SAME_PIXEL(), which tests for equality, may need to be redefined too
   (e.g., if MI_PIXEL_TYPE is a struct or a union). */
#ifdef MI_PIXEL_TYPE
typedef MI_PIXEL_TYPE miPixel;
#else
typedef unsigned int miPixel;
#endif
#ifndef MI_SAME_PIXEL
#define MI_SAME_PIXEL(pixel1,pixel2) \
  ((pixel1) == (pixel2))
#endif


/**********************************************************************/

/* A miPaintedSet is an opaque structure that contains a set of painted
   points, i.e., a set of points partitioned according to the pixel color
   used for painting.  When any public drawing function is invoked, a
   pointer to a miPaintedSet is passed as its first argument. */
typedef struct lib_miPaintedSet miPaintedSet;

/* Constructor and destructor for the miPaintedSet class. */
extern miPaintedSet * miNewPaintedSet ___P((void));
extern void miDeletePaintedSet ___P((miPaintedSet *paintedSet));

/* A function that clears any miPaintedSet (i.e. makes it the empty set). */
extern void miClearPaintedSet ___P((miPaintedSet *paintedSet));

/**********************************************************************/

/* A miGC is an opaque structure that contains high-level drawing
   parameters.  When any public drawing function is invoked, a pointer to a
   miGC is passed as its second argument. */
typedef struct lib_miGC miGC;

/* Constructor, destructor, and copy constructor for the miGC class. */
extern miGC * miNewGC ___P((int npixels, ___const miPixel *pixels)); /* npixels >= 2 */
extern void miDeleteGC ___P((miGC *pGC));
extern miGC * miCopyGC ___P((___const miGC *pGC));

/* Values for an miGC's miGCLineStyle attribute (default=MI_LINE_SOLID). */
enum { MI_LINE_SOLID, MI_LINE_ON_OFF_DASH, MI_LINE_DOUBLE_DASH };

/* Values for an miGC's miGCJoinStyle attribute (default=MI_JOIN_MITER). */
enum { MI_JOIN_MITER, MI_JOIN_ROUND, MI_JOIN_BEVEL, MI_JOIN_TRIANGULAR };

/* Values for an miGC's miGCCapStyle attribute (default=MI_CAP_BUTT).
   MI_CAP_NOT_LAST is the same as MI_CAP_BUTT except when drawing
   zero-width (Bresenham) polylines; it causes the final pixel not to be
   drawn.  A polyline drawn in this way is called `continuable'. */
enum { MI_CAP_NOT_LAST, MI_CAP_BUTT, MI_CAP_ROUND, MI_CAP_PROJECTING, MI_CAP_TRIANGULAR };

/* Values for an miGC's miGCFillRule attribute (default=MI_EVEN_ODD_RULE). */
enum { MI_EVEN_ODD_RULE, MI_WINDING_RULE };

/* Values for an miGC's miGCArcMode attribute (default=MI_ARC_PIE_SLICE). */
enum { MI_ARC_CHORD, MI_ARC_PIE_SLICE };

/* Possibilities for the `attribute' argument of miSetGCAttrib.  (All the
   preceding, plus MI_GC_LINE_WIDTH.) */
typedef enum { MI_GC_FILL_RULE, MI_GC_JOIN_STYLE, MI_GC_CAP_STYLE, MI_GC_LINE_STYLE, MI_GC_ARC_MODE, MI_GC_LINE_WIDTH } miGCAttribute;

/* A function that sets a single integer-valued miGC attribute.  `value'
   must be one of the preceding enum's, except when
   attribute=MI_GC_LINE_WIDTH, in which case value>=0 is required. */
extern void miSetGCAttrib ___P((miGC *pGC, miGCAttribute attribute, int value));

/* A function that sets a list of integer-value miGC attributes. */
extern void miSetGCAttribs ___P((miGC *pGC, int nattributes, ___const miGCAttribute *attributes, ___const int *values));

/* Functions that set miGC attributes that are not integer-valued.
   Note: currently, `offset' must be nonnegative. */
extern void miSetGCDashes ___P((miGC *pGC, int ndashes, ___const unsigned int *dashes, int offset));
extern void miSetGCMiterLimit ___P((miGC *pGC, double miter_limit));
extern void miSetGCPixels ___P((miGC *pGC, int npixels, ___const miPixel *pixels)); /* npixels >=2 */

/* Additional functions that set miGC attributes: in particular, functions
   that set the paint style that will be used.  Only in the case of `solid'
   painting (the default) is the above pixel array relevant. */
extern void miSetGCPaintSolid ___P((void));
extern void miSetGCPaintInterpParallel ___P((miPoint pts[2], miPixel pixels[2]));
extern void miSetGCPaintInterpTriangular ___P((miPoint pts[3], miPixel pixels[3]));
extern void miSetGCPaintInterpElliptical ___P((void));

/*********** DECLARATIONS OF PUBLIC DRAWING FUNCTIONS ******************/

/* The semantics of these drawing functions is similar to that of the
   corresponding X11 drawing functions.  Wide polylines (polylines with
   line width >= 1) are treated as polygons to be rendered by filling.
   Zero-width polylines are not invisible: instead, they are single-pixel
   polylines, specially rendered by the Bresenham midpoint line algorithm.

   Also, adjoining polygons (those with an edge in common) are drawn
   without gaps.  To arrange this, the `right' and `bottom' edges of any
   polygon are not drawn when the polygon is filled.  The filling of
   rectangles is similar.

   Wide arcs and polyarcs are drawn with a circular brush, of diameter
   equal to the line width.  Every brushed pixel is painted.  Zero-width
   arcs and polyarcs are not invisible: instead, they are single-pixel
   arcs, specially rendered by the Bresenham `midpoint arc' algorithm. */

/* For consistency, the first three arguments of each drawing function are
   (1) a pointer to a miPaintedSet, and (2) a pointer to an miGC. */

/* 1. Drawing functions for points, polylines, and polygons.

   The final three arguments of each are a `coordinate mode' (see below), a
   specified number of points, and an array that contains the points.
   miDrawPoints draws a cloud of points, miDrawLines draws a polyline, and
   miFillPolygon draws a filled polygon. */

/* Possible values for the `coordinate mode' argument (specifying whether
   the points in the points array, after the first point, are given in
   absolute or relative coordinates). */
typedef enum { MI_COORD_MODE_ORIGIN, MI_COORD_MODE_PREVIOUS } miCoordMode;

/* Possible values for the `shape' argument of miFillPolygon().  Two
   possibilities: (1) general (i.e., not necessarily convex, with
   self-intersections allowed), or (2) convex and not self-intersecting.
   Latter case can be drawn more rapidly. */
typedef enum { MI_SHAPE_GENERAL, MI_SHAPE_CONVEX } miPolygonShape;

___BEGIN_DECLS

extern void miDrawPoints ___P((miPaintedSet *paintedSet, ___const miGC *pGC, miCoordMode mode, int npts, ___const miPoint *pPts));
extern void miDrawLines ___P((miPaintedSet *paintedSet, ___const miGC *pGC, miCoordMode mode, int npts, ___const miPoint *pPts));
extern void miFillPolygon ___P((miPaintedSet *paintedSet, ___const miGC *pGC, miPolygonShape shape, miCoordMode mode, int npts, ___const miPoint *pPts));

/* 2. Rectangle-related drawing functions.

   These draw and fill a specified number of rectangles, supplied as an
   array of miRectangles. */

/* Structure that defines a rectangle.  Upper left corner is [x,y] and
   lower right corner is [x+width,y+height]. */
typedef struct
{
  int x, y;			/* upper left corner */
  unsigned int width, height;	/* width >= 1 and height >= 1 */
} miRectangle;

extern void miDrawRectangles ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int nrects, ___const miRectangle *pRects));
extern void miFillRectangles ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int nrects, ___const miRectangle *pRects));

/* 3. Arc-related drawing functions.

   Each of these takes as argument a multi-arc, i.e. an array of elliptic
   arcs.  Here, an `elliptic arc' is a piece of an ellipse whose axes are
   aligned with the coordinate axes.  The arcs are not required to be
   contiguous.

   miDrawArcs draws a multi-arc.  If the arcs are contiguous, they will be
   joined as specified in the miGC.  Note that miDrawArcs is not reentrant,
   i.e., not thread-safe (for a thread-safe variant, see below).

   miFillArcs draws a sequence of filled arcs.  They are filled either as
   chords or as pie slices, as specified by the graphics context. */

/* Structure that defines an `arc' (i.e. a segment of an ellipse whose
   principal axes are aligned with the coordinate axes).  The upper left
   corner of the bounding box is [x,y], and the lower right corner is
   [x+width,y+height].  By convention, angle1 and angle2 are the starting
   polar angle and angle range that the arc would have if it were scaled
   into a circular arc. */
typedef struct
{
  int x, y;		/* upper left corner of ellipse's bounding box */
  unsigned int width, height;	/* dimensions; width, height >= 1 */
  int angle1, angle2;	/* starting angle and angle range, in 1/64 degrees */
} miArc;

extern void miDrawArcs ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int narcs, ___const miArc *parcs));
extern void miFillArcs ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int narcs, ___const miArc *parcs));

/* 4. A reentrant (thread-safe) arc-drawing function.  A special function
   is necessary because the normal arc-drawing function miDrawArcs
   maintains an internal, fixed-size cache of rasterized ellipses, one for
   each arc that is drawn.  The presence of this persistent data (internal
   to libxmi) prevents miDrawArcs from being reentrant.  miDrawArcs_r is a
   reentrant substitute.

   The caller of miDrawArcs_r must supply a pointer to an miEllipseCache
   object as the final argument.  A pointer to such an object, which is
   opaque, is returned by miNewEllipseCache.  After zero or more calls to
   miDrawArcs_r, the object may be deleted by calling
   miDeleteEllipseCache. */

typedef struct lib_miEllipseCache miEllipseCache;
extern miEllipseCache * miNewEllipseCache ___P((void));
extern void miDeleteEllipseCache ___P((miEllipseCache *ellipseCache));

extern void miDrawArcs_r ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int narcs, ___const miArc *parcs, miEllipseCache *ellipseCache));

___END_DECLS

/***************** LIBXMI's Canvas-Painting ***********************/

/* A miCanvas encapsulates (i) a drawable, which is a miCanvasPixmap, and
   (ii) parameters that specify how pixels should be painted.  By default,
   a miCanvasPixmap is a miPixmap, i.e., basically a 2-D array of miPixels
   (an array of pointers to rows of miPixels).  That is a low-level
   implementation decision that may easily be changed by the libxmi
   installer. */

/* Binary pixel-merging function type.  Such a function maps a source pixel
   and a destination pixel to a new, merged pixel. */
typedef miPixel (*miPixelMerge2) ___P((miPixel source, miPixel destination));

/* Ternary pixel-merging function type.  Such a function maps a texture
   pixel, a source pixel, and a destination pixel, to a new, merged
   pixel. */
typedef miPixel (*miPixelMerge3) ___P((miPixel texture, miPixel source, miPixel destination));

/* Definitions of miBitmap and miPixmap.  By convention, (0,0) is upper
   left hand corner. */
typedef struct
{
  int **bitmap;			/* each element is 0 or 1 */
  unsigned int width;
  unsigned int height;
}
miBitmap;

typedef struct
{
  miPixel **pixmap;		/* each element is a miPixel */
  unsigned int width;
  unsigned int height;
}
miPixmap;

/* Definition of miCanvasPixmap, the datatype of the drawable encapsulated
   within a miCanvas.  By default, a miCanvasPixmap is a miPixmap.  The
   libxmi installer may alter the definition by defining the symbol
   MI_CANVAS_DRAWABLE_TYPE at installation time. */
#ifdef MI_CANVAS_DRAWABLE_TYPE
typedef MI_CANVAS_DRAWABLE_TYPE miCanvasPixmap;
#else
typedef miPixmap miCanvasPixmap;
#endif

/* Definition of the miCanvas structure. */
typedef struct
{
  /* Drawable. */
  miCanvasPixmap *drawable;

  /* A stipple.  (Default is NULL, which means no stipping.  If non-NULL,
     the canvas will be tiled with the stipple, and painting will be
     allowed to take place only at points where the stipple is nonzero.) */
  miBitmap *stipple;
  miPoint stippleOrigin;   /* upper left corner of mask is mapped to this */

  /* A texture.  (Default is NULL, which means no texturing.  If non-NULL,
     the canvas will be tiled with the texture, and painting of a pixel at
     any point will be affected by the value of the texture pixel there.)  */
  miPixmap *texture;
  miPoint textureOrigin;   /* upper left corner of texture is mapped to this */

  /* User-specified binary pixel-merging function, if any.  (Default is
     NULL, which means the Painter's Algorithm is used: source pixel will
     replace destination pixel.) */
  miPixelMerge2 pixelMerge2;

  /* User-specified ternary pixel-merging function, if any.  Used when a
     texture has been specified.  (Default is NULL, which means the
     Painter's Algorithm is used: texture pixel will replace destination
     pixel, and source pixel will be ignored.) */
  miPixelMerge3 pixelMerge3;

} miCanvas;

/* The public function that merges pixels from a miPaintedSet onto a
   miCanvas.  `origin' is the point on the miCanvas to which the point
   (0,0) in the miPaintedSet is mapped.  (It could be called `offset'.) */
extern void miCopyPaintedSetToCanvas ___P((const miPaintedSet *paintedSet, miCanvas *canvas, miPoint origin));

/* If MI_CANVAS_DRAWABLE_TYPE is defined by the libxmi installer (see
   above), then the accessor macros MI_GET_CANVAS_DRAWABLE_PIXEL() and
   MI_SET_CANVAS_DRAWABLE_PIXEL() will also need to be defined.  The
   default accessor macros simply access the 2-D miPixel array within a
   miPixmap.  MI_GET_CANVAS_DRAWABLE_BOUNDS() should be defined too. */
#ifndef MI_GET_CANVAS_DRAWABLE_PIXEL
#define MI_GET_CANVAS_DRAWABLE_PIXEL(pCanvas, x, y, pixel) \
    (pixel) = (pCanvas)->drawable->pixmap[(y)][(x)];
#endif
#ifndef MI_SET_CANVAS_DRAWABLE_PIXEL
#define MI_SET_CANVAS_DRAWABLE_PIXEL(pCanvas, x, y, pixel) \
    (pCanvas)->drawable->pixmap[(y)][(x)] = (pixel);
#endif
#ifndef MI_GET_CANVAS_DRAWABLE_BOUNDS
#define MI_GET_CANVAS_DRAWABLE_BOUNDS(pCanvas, xleft, ytop, xright, ybottom) \
    { (xleft) = 0; (ytop) = 0; \
      (xright) = (pCanvas)->drawable->width - 1; \
      (ybottom) = (pCanvas)->drawable->height - 1; \
    }
#endif

/* Functions that set data elements of a miCanvas. */
extern void miSetCanvasStipple ___P((miCanvas *pCanvas, ___const miBitmap *pStipple, miPoint stippleOrigin));
extern void miSetCanvasTexture ___P((miCanvas *pCanvas, ___const miPixmap *pTexture, miPoint textureOrigin));

/* Functions that set the binary and ternary pixel-merging functions to be
   used when pixels from a miPaintedSet are applied to a miCanvas.  The
   defaults are NULL; for the meaning of NULL, see above. */
extern void miSetPixelMerge2 ___P((miCanvas *pCanvas, miPixelMerge2 pixelMerge2));
extern void miSetPixelMerge3 ___P((miCanvas *pCanvas, miPixelMerge3 pixelMerge3));

/* The libxmi installer may request that the default algorithm used when
   applying pixels to a miPaintCanvas be something other than the Painter's
   Algorithm, by defining MI_DEFAULT_MERGE2_PIXEL at installation time. */
#ifndef MI_DEFAULT_MERGE2_PIXEL
/* use painter's algorithm */
#define MI_DEFAULT_MERGE2_PIXEL(new, source, dest) { (new) = (source); }
#endif

/* Likewise, the libxmi installer may request that the default algorithm
   used when applying pixels to a miPaintCanvas, when a texture pixel is
   available, be something other than the `replace canvas pixel by texture
   pixel' algorithm. */
#ifndef MI_DEFAULT_MERGE3_PIXEL
/* use painter's algorithm */
#define MI_DEFAULT_MERGE3_PIXEL(new, texture, source, dest) { (new) = (texture); }
#endif

#ifndef MI_CANVAS_DRAWABLE_TYPE
/* Constructor, destructor, and copy constructor for the miCanvas class.
   These are declared (and defined) only if the libxmi installer doesn't
   redefine the type of the drawable encapsulated within a miCanvas. */
extern miCanvas * miNewCanvas ___P((unsigned int width, unsigned int height, miPixel initPixel));
extern void miDeleteCanvas ___P((miCanvas *pCanvas));
extern miCanvas * miCopyCanvas ___P((___const miCanvas *pCanvas));
#endif /* not MI_CANVAS_DRAWABLE_TYPE */

/**********************************************************************/

#endif /* not _XMI_H_ */
