/* Wrappers for standard storage allocation functions.  The tests for zero
   size, etc., are necessitated by the way in which the original X11
   scan-conversion code was written. */

#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_api.h"

/* wrapper for malloc() */
voidptr_t 
#ifdef _HAVE_PROTOS
mi_xmalloc (size_t size)
#else
mi_xmalloc (size)
     size_t size;
#endif
{
  voidptr_t p;

  if (size == 0)
    return (voidptr_t)NULL;

  p = (voidptr_t) malloc (size);
  if (p == (voidptr_t)NULL)
    {
      fprintf (stderr, "libxmi: ");
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}

/* wrapper for calloc() */
voidptr_t 
#ifdef _HAVE_PROTOS
mi_xcalloc (size_t nmemb, size_t size)
#else
mi_xcalloc (nmemb, size)
     size_t nmemb, size;
#endif
{
  voidptr_t p;

  if (size == 0)
    return (voidptr_t)NULL;

  p = (voidptr_t) calloc (nmemb, size);
  if (p == (voidptr_t)NULL)
    {
      fprintf (stderr, "libxmi: ");
      perror ("out of memory");
      exit (EXIT_FAILURE);
    }
  return p;
}

/* wrapper for realloc() */
voidptr_t 
#ifdef _HAVE_PROTOS
mi_xrealloc (voidptr_t p, size_t size)
#else
mi_xrealloc (p, size)
     voidptr_t p;
     size_t size;
#endif
{
  if (!p)
    return mi_xmalloc (size);
  else
    {
      if (size == 0)
	{
	  free (p);
	  return (voidptr_t)NULL;
	}
      
      p = (voidptr_t) realloc (p, size);
      if (p == (voidptr_t)NULL)
	{
	  fprintf (stderr, "libxmi: ");
	  perror ("out of memory");
	  exit (EXIT_FAILURE);
	}
      return p;
    }
}
