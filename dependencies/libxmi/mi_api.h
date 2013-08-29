/* Internal counterparts of libxmi's core functions, each of which
   takes a (miPaintedSet *) as first argument. */

extern void miDrawPoints_internal ___P((miPaintedSet *paintedSet, ___const miGC *pGC, miCoordMode mode, int npts, ___const miPoint *pPts));
extern void miDrawLines_internal ___P((miPaintedSet *paintedSet, ___const miGC *pGC, miCoordMode mode, int npts, ___const miPoint *pPts));
extern void miFillPolygon_internal ___P((miPaintedSet *paintedSet, ___const miGC *pGC, miPolygonShape shape, miCoordMode mode, int npts, ___const miPoint *pPts));
extern void miDrawRectangles_internal ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int nrects, ___const miRectangle *pRects));
extern void miFillRectangles_internal ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int nrects, ___const miRectangle *pRects));
extern void miDrawArcs_internal ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int narcs, ___const miArc *parcs));
extern void miFillArcs_internal ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int narcs, ___const miArc *parcs));
extern void miDrawArcs_r_internal ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int narcs, ___const miArc *parcs, miEllipseCache *ellipse_cache));

/* Internal functions, which are called by wrapper functions defined in
   mi_api.c. */

extern void miWideDash ___P((miPaintedSet *paintedSet, ___const miGC *pGC, miCoordMode mode, int npts, ___const miPoint *pPts));
extern void miZeroDash ___P((miPaintedSet *paintedSet, ___const miGC *pGC, miCoordMode mode, int npts, ___const miPoint *pPts));
extern void miWideLine ___P((miPaintedSet *paintedSet, ___const miGC *pGC, miCoordMode mode, int npts, ___const miPoint *pPts));
extern void miZeroLine ___P((miPaintedSet *paintedSet, ___const miGC *pGC, miCoordMode mode, int npts, ___const miPoint *pPts));

extern void miPolyArc ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int narcs, ___const miArc *parcs));
extern void miZeroPolyArc ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int narcs, ___const miArc *parcs));

extern void miPolyArc_r ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int narcs, ___const miArc *parcs, miEllipseCache *ellipse_cache));
extern void miZeroPolyArc_r ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int narcs, ___const miArc *parcs, miEllipseCache *ellipse_cache));

/* Declarations of other internal functions, which should really be moved
   elsewhere. */

/* wrappers for storage allocation functions, see mi_alloc.c */
extern voidptr_t mi_xmalloc ___P((size_t size));
extern voidptr_t mi_xcalloc ___P((size_t nmemb, size_t size));
extern voidptr_t mi_xrealloc ___P((voidptr_t p, size_t size));

/* other misc. internal functions */
extern void miFillConvexPoly ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int count, ___const miPoint *ptsIn));
extern void miFillGeneralPoly ___P((miPaintedSet *paintedSet, ___const miGC *pGC, int count, ___const miPoint *ptsIn));
extern void miStepDash ___P((int dist, int *pDashNum, int *pDashIndex, ___const unsigned int *pDash, int numInDashList, int *pDashOffset));
