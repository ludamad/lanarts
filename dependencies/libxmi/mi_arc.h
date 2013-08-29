#ifndef NO_NONREENTRANT_POLYARC_SUPPORT
/* The reentrant functions miPolyArc_r() and miZeroPolyArc_r() take a
   pointer to a user-supplied cache of rasterized elliptical arcs as an
   argument.  Their non-reentrant counterparts miPolyArc() and
   miZeroPolyArc() use a pointer to an in-library cache. */
extern miEllipseCache * _mi_ellipseCache;
#endif /* NO_NONREENTRANT_POLYARC_SUPPORT */
