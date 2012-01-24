/* permissive-fov.h */


/* Copyright (c) 2007, Jonathon Duerig. Licensed under the BSD
   license. See LICENSE.txt for details. */

#ifndef PERMISSIVE_FOV_H_DUERIG
#define PERMISSIVE_FOV_H_DUERIG

/* Usage: Call permissiveSquareFov() below to calculate fov for a
   particular radius.

   permissiveFov() and the functions for manipulating
   permissiveMaskT provide a more flexible method of determining
   shapes and distance for visitation.
*/

/* This library is *NOT* thread safe. In particular, no
   synchronization primitves are used, and writes to masks are not
   atomic. */

/* This library is re-entrant. */

#ifdef __cplusplus
extern "C"
{
#endif

/* See below for a description of how to use this struct */
typedef struct
{
  /* Do not interact with the members directly.
     Use the provided functions. */
  int north;
  int south;
  int east;
  int west;
  int width;
  int height;
  unsigned int * mask;
} permissiveMaskT;

/* Function specifications for the two user functions called by
   permissiveFov(). */

/* isBlockedFunction() may be called even if a square will not be
   visited. isBlockedFunction() may be called more than once.  */
typedef int (*isBlockedFunction)(short destX, short destY, void * context);

/* visitFunction() will be called at most one time. visitFunction()
   will only be called if a mask allows visitation for that square. */
typedef void (*visitFunction)(short destX, short destY, void * context);

/*  Calculate precise permissive field of view sourced from the point
    (sourceX, sourceY). */
/*

  radius -- The distance in a square which will be visited. A radius
            of 0 will visit only the source square. A radius of n will
            visit the n*2+1 by n*2+1 square centered on the source.
  isBlocked() -- called to determine whether a particular square
                 blocks visibility. It may be called more than once on
                 a particular square. It will always be called at
                 least once if a square is considered visible. If the
                 source square is considered blocked, the result is
                 disregarded. context is passed unmodified to both
                 isBlocked() and visit().
  visit() -- called when a square is determined to be visible. It will
             be called exactly once on each visible square. context is
             passed unmodified to both isBlocked() and visit().
  context -- user-defined data which is passed to both isBlocked() and
             visit().
*/
void permissiveSquareFov(short sourceX, short sourceY,
                         int radius,
                         isBlockedFunction isBlocked,
                         visitFunction visit, void * context);

/* Similar to permissiveSquareFov() except that a mask is provided
   rather than a simple radius.

   mask provides a visitation mask and determines the radius to be
   visited. See below for a more complete desription of how it works.
*/
void permissiveFov(short sourceX, short sourceY,
                   permissiveMaskT * mask,
                   isBlockedFunction isBlocked,
                   visitFunction visit, void * context);

typedef enum
{
  PERMISSIVE_NO_FAILURE,
  PERMISSIVE_OUT_OF_MEMORY,
  PERMISSIVE_FAILED_TO_OPEN_FILE,
  PERMISSIVE_LOAD_NO_ORIGIN,
  PERMISSIVE_LOAD_INVALID_CHARACTER,
  PERMISSIVE_SAVE_WRITE_FAILED,
} permissiveErrorT;

/*
  The struct permissiveMaskT is used to specify the area around the
  source square which should be visited and the shape of that
  square. It provides a way to specify a (potentially asymmetric) box
  around the player which is used to bound the computation of FoV. And
  then it provides a way to specify which squares inside of that box
  to actually visit. This can be used to, for instance, provide
  different FoV depending on facing, or to cause FoV to be bounded by
  an approximation of a circle rather than a rectangle.

  Even if a map square has been marked as 'do not visit', that square
  passes visibility (or not) as normal, and isBlocked() may be called
  on it.
*/

/* Use one of these two functions to initialize a mask. Use only one
/* of the functions. */

/* Create a mask of the proper dimensions initialized to allow visits
/* of every square. The origin is the player. */
/* mask -- a pointer to an uninitialized permissiveMaskT */
/* north, south, east, west -- The distance in the given direction
/*                             from the origin which will be
/*                             visited. A value of 0 means that only
/*                             the row (or column) of the origin will
/*                             be visited. A value of 1 means that the
/*                             origin row (or column) and the one
/*                             adjascent to it in the direction
/*                             specified will be visited,
/*                             etc. Quadrant I is northeast, Quadrant
/*                             II is northwest, Quadrant III is
/*                             southwest, and Quadrant IV is
/*                             southeast. */
/* result -- A value representing why it failed. */
permissiveErrorT initPermissiveMask(permissiveMaskT * mask, int north,
                                    int south, int east, int west);

/* Create a mask of the proper dimensions initialized by reading from
   a file. */
/* mask -- a pointer to an uninitialized permissiveMaskT
   fileName -- a file which will be read in its entirety to determine
               a mask. The file should be plain text, divided into
               lines. The first line is the northmost and the last is
               the southmost. Each line should consist of one of
               following characters:

              '!' or '@' -- The origin of the mask. This character
                            should occur only once. The dimensions of
                            the resulting mask are determined by the
                            relative location of this character. Only
                            one of the two characters should be
                            encountered. '@' is used when the origin
                            should be visited, and '!' is used when
                            the origin should not be visited.
              '.' -- Squares marked with a '.' are visited as normal.
              '#' -- Squares marked with a '#' are not visited.

              If some lines have more characters than others, the
              maximum size line is taken to be overall width of the
              mask, and the other lines are filled in with '#'
              characters.
*/
permissiveErrorT loadPermissiveMask(permissiveMaskT * mask,
                                    char const * fileName);

/* Clean up resources used by an initialized mask. */
void cleanupPermissiveMask(permissiveMaskT * mask);

/* Save permissive mask in the format described above. */
permissiveErrorT savePermissiveMask(permissiveMaskT * mask,
                                    char const * fileName);

/* Specify that a square should be visited normally
   (setPermissiveMask), or that is should not be visited
   (clearPermissiveVisit). x and y are relative to the origin
   (player). */
void setPermissiveVisit(permissiveMaskT * mask, int x, int y);
void clearPermissiveVisit(permissiveMaskT * mask, int x, int y);

/* Check to see whether a square will be visited normally. 1 is
   returned if the square will be visited normally, and 0 if the
   square will not be visited. */
int doesPermissiveVisit(permissiveMaskT * mask, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
