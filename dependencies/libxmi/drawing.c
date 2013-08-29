/* This is a sample program that uses libxmi to draw on a 60x35 canvas, and
   writes the canvas to standard output.  Actually, since libxmi uses a
   two-stage graphics pipeline, the program first constructs a `painted
   set' (a set of points with integer coordinates, partitioned by pixel
   value), and merges the painted set onto a 60x35 canvas.

   The painted set consists of a polyline and an arc that subtends 270
   degrees.  The polyline crosses the arc, and sticks into its interior.
   Both the polyline and the arc are dashed.  The line type is
   MI_LINE_ON_OFF_DASHED, and the `on' dashes are multicolored: they cycle
   through colors 2,3,4.  (The `off' dashes are not drawn, but if they were
   [which would be the case if MI_LINE_ON_OFF_DASHED were replaced by
   MI_LINE_DOUBLE_DASHED], they would all be in color 1.)  The background
   color of the canvas onto which the painted set is merged is 0.

   All these color values are miPixel values, and may be chosen
   arbitrarily.  By default, miPixel is typedef'd to `unsigned int'.  You
   may interpret a miPixel however you choose.  It could be a color index,
   an RGB value,...

   The line width for both the polyline and the arc is set to `0'.  This is
   a special value: it requests that a so-called Bresenham algorithm be
   used.  A Bresenham algorithm often yields better-looking lines than line
   width 1.  If the line width is positive, the polyline and the arc will
   be drawn with a brush of that width, and all pixels touched by the brush
   will be painted.  Dashing may be requested for positive line width, just
   as it is for line width 0. */

#include <stdio.h>
#include <stdlib.h>
#include <xmi.h>            /* public libxmi header file */

int main ()
{
  miPoint points[4];        /* 3 line segments in the polyline */
  miArc arc;                /* 1 arc to be drawn */
  miPixel pixels[4];        /* pixel values for drawing and dashing */
  unsigned int dashes[2];   /* length of `on' and `off' dashes */
  miGC *pGC;                /* graphics context */
  miPaintedSet *paintedSet; /* opaque object to be painted */
  miCanvas *canvas;         /* drawing canvas (including pixmap) */
  miPoint offset;           /* for miPaintedSet -> miCanvas transfer */
  int i, j;
     
  /* define polyline: vertices are (25,5) (5,5), (5,25), (35,22) */
  points[0].x = 25;  points[0].y = 5;
  points[1].x = 5;   points[1].y = 5;
  points[2].x = 5;   points[2].y = 25;
  points[3].x = 35;  points[3].y = 22;
     
  /* define elliptic arc */
  arc.x = 20; arc.y = 15;   /* upper left corner of bounding box */
  arc.width = 30;           /* x range of box: 20..50 */
  arc.height = 16;          /* y range of box: 15..31 */
  arc.angle1 = 0 * 64;      /* starting angle (1/64'ths of a degree) */
  arc.angle2 = 270 * 64;    /* angle range (1/64'ths of a degree) */
     
  /* create and modify graphics context */
  pixels[0] = 1;            /* pixel value for `off' dashes, if drawn */
  pixels[1] = 2;            /* default pixel for drawing */
  pixels[2] = 3;            /* another pixel, for multicolored dashes */
  pixels[3] = 4;            /* another pixel, for multicolored dashes */
  dashes[0] = 4;            /* length of `on' dashes */
  dashes[1] = 2;            /* length of `off' dashes */
  pGC = miNewGC (4, pixels);
  miSetGCAttrib (pGC, MI_GC_LINE_STYLE, MI_LINE_ON_OFF_DASH);
  miSetGCDashes (pGC, 2, dashes, 0);
  miSetGCAttrib (pGC, MI_GC_LINE_WIDTH, 0); /* Bresenham algorithm */
     
  /* create empty painted set */
  paintedSet = miNewPaintedSet ();
     
  /* paint dashed polyline and dashed arc onto painted set */
  miDrawLines (paintedSet, pGC, MI_COORD_MODE_ORIGIN, 4, points);
  miDrawArcs (paintedSet, pGC, 1, &arc);
     
  /* create 60x35 canvas (initPixel=0); merge painted set onto it */
  canvas = miNewCanvas (60, 35, 0);
  offset.x = 0; offset.y = 0;
  miCopyPaintedSetToCanvas (paintedSet, canvas, offset);
     
  /* write canvas's pixmap (a 60x35 array of miPixels) to stdout */
  for (j = 0; j < canvas->drawable->height; j++)
    {
      for (i = 0; i < canvas->drawable->width; i++)
	/* note: column index precedes row index */
	printf ("%d", canvas->drawable->pixmap[j][i]);
      printf ("\n");
    }
     
  /* clean up */
  miDeleteCanvas (canvas);
  miDeleteGC (pGC);
  miClearPaintedSet (paintedSet); /* not necessary */
  miDeletePaintedSet (paintedSet);
     
  return 0;
}
