/* A miGC structure contains high-level drawing parameters.  Such
   structures are created, modified, and destroyed by functions in mi_gc.c.
   miGC is typedef'd as lib_miGC in xmi.h.  lib_miGC is defined here,
   privately, so that an miGC will be opaque.

   The chief difference between libxmi and X11 is that libxmi supports
   painting with interpolated colors (e.g., `gradient fill').  Several
   types of interpolation are supported.

   Other significant differences from X11 are (1) the dash array is an
   array of unsigned int's rather than char's, so that much longer dashes
   may be drawn, and (2) the miter limit is a GC attribute like any other
   (in X11, it is fixed at 10.43, and may not be altered). */

/* Values for an miGC's miGCPaintStyle attribute (default=MI_PAINT_SOLID). */
enum { MI_PAINT_SOLID, MI_PAINT_INTERPOLATED_PARALLEL, MI_PAINT_INTERPOLATED_TRIANGULAR, MI_PAINT_INTERPOLATED_ELLIPTICAL, MI_PAINT_CUSTOM };

struct lib_miGC
{
  /* paint style (either solid or an interpolated [`gradient'] style) */
  int paintStyle;		/* default = miPaintSolid */

  /* array of pixel types, used if paintStyle = miPaintSolid */
  miPixel *pixels;		/* array of pixel types */
  int numPixels;		/* number of pixel types (must be >=2) */

  /* arrays of pixel types, used if paintStyle is an interpolated style */
  miPixel triangularInterpPixels[3];
  miPixel parallelInterpPixels[2];
  miPixel ellipticalInterpPixels[3];

  /* parameters for libxmi's core drawing functions (dash-related) */
  unsigned int *dash;		/* dash array (lengths of dashes in pixels) */
  int numInDashList;		/* length of dash array */
  int dashOffset;		/* pixel offset of first dash (nonnegative) */

  /* parameters for libxmi's core drawing functions (not dash-related) */
  int lineStyle;		/* default = miLineSolid */
  unsigned int lineWidth;	/* line thickness in pixels (default = 0) */
  int joinStyle;		/* default = miJoinMiter */
  int capStyle;			/* default = miCapButt */
  double miterLimit;		/* default = 10.43, as in X11 */

  /* parameters for libxmi's core filling functions */
  int fillRule;			/* default = miEvenOddRule */
  int arcMode;			/* default = miArcPieSlice */
};
