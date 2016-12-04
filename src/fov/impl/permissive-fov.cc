// permissive-fov.cc

/* Copyright (c) 2007, Jonathon Duerig. Licensed under the BSD
   license. See LICENSE.txt for details. */

#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

#include "permissive-fov.h"

using std::list;
using std::max;
using std::min;
using std::string;

namespace
{
  struct offsetT
  {
  public:
    offsetT(short newX = 0, short newY = 0)
      : x(newX)
        , y(newY)
    {
    }
  public:
    short x;
    short y;
  };

  std::ostream & operator<<(std::ostream & stream, offsetT const & right)
  {
    stream << "(" << right.x << ", " << right.y << ")";
    return stream;
  }

  struct fovStateT
  {
    offsetT source;
    permissiveMaskT * mask;
    isBlockedFunction isBlocked;
    visitFunction visit;
    void * context;

    offsetT quadrant;
    offsetT extent;
  };

  struct lineT
  {
    lineT(offsetT newNear=offsetT(), offsetT newFar=offsetT())
      : near(newNear)
      , far(newFar)
    {
    }

    bool isBelow(offsetT const & point)
    {
      return relativeSlope(point) > 0;
    }

    bool isBelowOrContains(offsetT const & point)
    {
      return relativeSlope(point) >= 0;
    }

    bool isAbove(offsetT const & point)
    {
      return relativeSlope(point) < 0;
    }

    bool isAboveOrContains(offsetT const & point)
    {
      return relativeSlope(point) <= 0;
    }

    bool doesContain(offsetT const & point)
    {
      return relativeSlope(point) == 0;
    }

    // negative if the line is above the point.
    // positive if the line is below the point.
    // 0 if the line is on the point.
    int relativeSlope(offsetT const & point)
    {
      return (far.y - near.y)*(far.x - point.x)
        - (far.y - point.y)*(far.x - near.x);
    }

    offsetT near;
    offsetT far;
  };

  struct bumpT
  {
    bumpT() : parent(NULL) {}
    offsetT location;
    bumpT * parent;
  };

  struct fieldT
  {
    fieldT() : steepBump(NULL), shallowBump(NULL) {}
    lineT steep;
    lineT shallow;
    bumpT * steepBump;
    bumpT * shallowBump;
  };

  void visitSquare(fovStateT const * state,
                   offsetT const & dest, list<fieldT>::iterator & currentField,
                   list<bumpT> & steepBumps,
                   list<bumpT> & shallowBumps,
                   list<fieldT> & activeFields);
  list<fieldT>::iterator checkField(list<fieldT>::iterator currentField,
                                    list<fieldT> & activeFields);
  void addShallowBump(offsetT const & point,
                      list<fieldT>::iterator currentField,
                      list<bumpT> & steepBumps,
                      list<bumpT> & shallowBumps);
  void addSteepBump(offsetT const & point,
                    list<fieldT>::iterator currentField,
                    list<bumpT> & steepBumps,
                    list<bumpT> & shallowBumps);
  bool actIsBlocked(fovStateT const * state, offsetT const & pos);

  void calculateFovQuadrant(fovStateT const * state)
  {
    list<bumpT> steepBumps;
    list<bumpT> shallowBumps;
    // activeFields is sorted from shallow-to-steep.
    list<fieldT> activeFields;
    activeFields.push_back(fieldT());
    activeFields.back().shallow.near = offsetT(0, 1);
    activeFields.back().shallow.far = offsetT(state->extent.x, 0);
    activeFields.back().steep.near = offsetT(1, 0);
    activeFields.back().steep.far = offsetT(0, state->extent.y);

    offsetT dest(0, 0);

    // Visit the source square exactly once (in quadrant 1).
    if (state->quadrant.x == 1 && state->quadrant.y == 1)
    {
      actIsBlocked(state, dest);
    }

    list<fieldT>::iterator currentField = activeFields.begin();
    short i = 0;
    short j = 0;
    int maxI = state->extent.x + state->extent.y;
    // For each square outline
    for (i = 1; i <= maxI && ! activeFields.empty(); ++i)
    {
      int startJ = max(0, i - state->extent.x);
      int maxJ = min(i, state->extent.y);
      // Visit the nodes in the outline
      for (j = startJ; j <= maxJ && currentField != activeFields.end(); ++j)
      {
        dest.x = i-j;
        dest.y = j;
        visitSquare(state, dest, currentField, steepBumps, shallowBumps,
                    activeFields);
      }
      currentField = activeFields.begin();
    }
  }

  void visitSquare(fovStateT const * state,
                   offsetT const & dest, list<fieldT>::iterator & currentField,
                   list<bumpT> & steepBumps,
                   list<bumpT> & shallowBumps,
                   list<fieldT> & activeFields)
  {
    // The top-left and bottom-right corners of the destination square.
    offsetT topLeft(dest.x, dest.y + 1);
    offsetT bottomRight(dest.x + 1, dest.y);
    while (currentField != activeFields.end()
           && currentField->steep.isBelowOrContains(bottomRight))
    {
      // case ABOVE
      // The square is in case 'above'. This means that it is ignored
      // for the currentField. But the steeper fields might need it.
      ++currentField;
    }
    if (currentField == activeFields.end())
    {
      // The square was in case 'above' for all fields. This means that
      // we no longer care about it or any squares in its diagonal rank.
      return;
    }

    // Now we check for other cases.
    if (currentField->shallow.isAboveOrContains(topLeft))
    {
      // case BELOW
      // The shallow line is above the extremity of the square, so that
      // square is ignored.
      return;
    }
    // The square is between the lines in some way. This means that we
    // need to visit it and determine whether it is blocked.
    bool isBlocked = actIsBlocked(state, dest);
    if (!isBlocked)
    {
      // We don't care what case might be left, because this square does
      // not obstruct.
      return;
    }

    if (currentField->shallow.isAbove(bottomRight)
        && currentField->steep.isBelow(topLeft))
    {
      // case BLOCKING
      // Both lines intersect the square. This current field has ended.
      currentField = activeFields.erase(currentField);
    }
    else if (currentField->shallow.isAbove(bottomRight))
    {
      // case SHALLOW BUMP
      // The square intersects only the shallow line.
      addShallowBump(topLeft, currentField, steepBumps, shallowBumps);
      currentField = checkField(currentField, activeFields);
    }
    else if (currentField->steep.isBelow(topLeft))
    {
      // case STEEP BUMP
      // The square intersects only the steep line.
      addSteepBump(bottomRight, currentField, steepBumps, shallowBumps);
      currentField = checkField(currentField, activeFields);
    }
    else
    {
      // case BETWEEN
      // The square intersects neither line. We need to split into two fields.
      list<fieldT>::iterator steeperField = currentField;
      list<fieldT>::iterator shallowerField = activeFields.insert(currentField, *currentField);
      addSteepBump(bottomRight, shallowerField, steepBumps, shallowBumps);
      checkField(shallowerField, activeFields);
      addShallowBump(topLeft, steeperField, steepBumps, shallowBumps);
      currentField = checkField(steeperField, activeFields);
    }
  }

  list<fieldT>::iterator checkField(list<fieldT>::iterator currentField,
                                    list<fieldT> & activeFields)
  {
    list<fieldT>::iterator result = currentField;
    // If the two slopes are colinear, and if they pass through either
    // extremity, remove the field of view.
    if (currentField->shallow.doesContain(currentField->steep.near)
        && currentField->shallow.doesContain(currentField->steep.far)
        && (currentField->shallow.doesContain(offsetT(0, 1))
            || currentField->shallow.doesContain(offsetT(1, 0))))
    {
      result = activeFields.erase(currentField);
    }
    return result;
  }

  void addShallowBump(offsetT const & point,
                      list<fieldT>::iterator currentField,
                      list<bumpT> & steepBumps,
                      list<bumpT> & shallowBumps)
  {
    // First, the far point of shallow is set to the new point.
    currentField->shallow.far = point;
    // Second, we need to add the new bump to the shallow bump list for
    // future steep bump handling.
    shallowBumps.push_back(bumpT());
    shallowBumps.back().location = point;
    shallowBumps.back().parent = currentField->shallowBump;
    currentField->shallowBump = & shallowBumps.back();
    // Now we have too look through the list of steep bumps and see if
    // any of them are below the line.
    // If there are, we need to replace near point too.
    bumpT * currentBump = currentField->steepBump;
    while (currentBump != NULL)
    {
      if (currentField->shallow.isAbove(currentBump->location))
      {
        currentField->shallow.near = currentBump->location;
      }
      currentBump = currentBump->parent;
    }
  }

  void addSteepBump(offsetT const & point,
                    list<fieldT>::iterator currentField,
                    list<bumpT> & steepBumps,
                    list<bumpT> & shallowBumps)
  {
    currentField->steep.far = point;
    steepBumps.push_back(bumpT());
    steepBumps.back().location = point;
    steepBumps.back().parent = currentField->steepBump;
    currentField->steepBump = & steepBumps.back();
    // Now look through the list of shallow bumps and see if any of them
    // are below the line.
    bumpT * currentBump = currentField->shallowBump;
    while (currentBump != NULL)
    {
      if (currentField->steep.isBelow(currentBump->location))
      {
        currentField->steep.near = currentBump->location;
      }
      currentBump = currentBump->parent;
    }
  }

  bool actIsBlocked(fovStateT const * state, offsetT const & pos)
  {
    offsetT adjustedPos(pos.x*state->quadrant.x + state->source.x,
                        pos.y*state->quadrant.y + state->source.y);
    bool result = state->isBlocked(adjustedPos.x, adjustedPos.y,
                                   state->context) == 1;
    if ((state->quadrant.x * state->quadrant.y == 1
         && pos.x == 0 && pos.y != 0)
        || (state->quadrant.x * state->quadrant.y == -1
            && pos.y == 0 && pos.x != 0)
        || doesPermissiveVisit(state->mask, pos.x*state->quadrant.x,
                               pos.y*state->quadrant.y) == 0)
    {
      return result;
    }
    else
    {
      state->visit(adjustedPos.x, adjustedPos.y, state->context);
      return result;
    }
  }
}

void permissiveSquareFov(short sourceX, short sourceY,
                         int inRadius, isBlockedFunction isBlocked,
                         visitFunction visit, void * context)
{
  int radius = max(inRadius, 0);
  permissiveMaskT mask;
  mask.north = radius;
  mask.south = radius;
  mask.east = radius;
  mask.west = radius;
  mask.width = 2*radius + 1;
  mask.height = 2*radius + 1;
  mask.mask = NULL;
  permissiveFov(sourceX, sourceY, &mask, isBlocked, visit, context);
}

void permissiveFov(short sourceX, short sourceY,
                   permissiveMaskT * mask, isBlockedFunction isBlocked,
                   visitFunction visit, void * context)
{
  fovStateT state;
  state.source = offsetT(sourceX, sourceY);
  state.mask = mask;
  state.isBlocked = isBlocked;
  state.visit = visit;
  state.context = context;

  static const int quadrantCount = 4;
  static const offsetT quadrants[quadrantCount] = {offsetT(1, 1),
                                                   offsetT(-1, 1),
                                                   offsetT(-1, -1),
                                                   offsetT(1, -1)};
  offsetT extents[quadrantCount] = {offsetT(mask->east, mask->north),
                                    offsetT(mask->west, mask->north),
                                    offsetT(mask->west, mask->south),
                                    offsetT(mask->east, mask->south)};
  int quadrantIndex = 0;
  for (; quadrantIndex < quadrantCount; ++quadrantIndex)
  {
    state.quadrant = quadrants[quadrantIndex];
    state.extent = extents[quadrantIndex];
    calculateFovQuadrant(&state);
  }
}

namespace
{
  static const int BITS_PER_INT = sizeof(int)*8;

  #define GET_INT(x,y) (((x)+(y)*mask->width)/BITS_PER_INT)
  #define GET_BIT(x,y) (((x)+(y)*mask->width)%BITS_PER_INT)

  unsigned int * allocateMask(int width, int height)
  {
    int cellCount = width * height;
    int intCount = cellCount / BITS_PER_INT;
    if (cellCount % BITS_PER_INT != 0)
    {
      ++intCount;
    }
    return new unsigned int[intCount];
  }
}

permissiveErrorT initPermissiveMask(permissiveMaskT * mask, int north,
                                    int south, int east, int west)
{
  permissiveErrorT result = PERMISSIVE_NO_FAILURE;
  mask->north = max(north, 0);
  mask->south = max(south, 0);
  mask->east = max(east, 0);
  mask->west = max(west, 0);
  mask->width = mask->west + 1 + mask->east;
  mask->height = mask->south + 1 + mask->north;
  mask->mask = allocateMask(mask->width, mask->height);
  if (mask->mask == NULL)
  {
    result = PERMISSIVE_OUT_OF_MEMORY;
  }
  return result;
}

permissiveErrorT loadPermissiveMask(permissiveMaskT * mask,
                                    char const * fileName)
{
  list<string> input;
  size_t maxLineSize = 1;
  std::ifstream file(fileName, std::ios::in);
  if (!file)
  {
    return PERMISSIVE_FAILED_TO_OPEN_FILE;
  }
  permissiveErrorT result = PERMISSIVE_NO_FAILURE;
  string line;
  getline(file, line);
  while (file)
  {
    maxLineSize = max(maxLineSize, line.size());
    input.push_front(line);
    getline(file, line);
  }
  mask->width = static_cast<int>(maxLineSize);
  mask->height = static_cast<int>(input.size());
  mask->mask = allocateMask(mask->width, mask->height);
  // TODO: Out of memory
  list<string>::iterator inputPos = input.begin();
  unsigned int * intPos = mask->mask;
  int bitPos = 0;
  for (int i = 0; i < mask->height; ++i, ++inputPos)
  {
    for (int j = 0; j < mask->width; ++j)
    {
      char current = '#';
      if (j < static_cast<int>(inputPos->size()))
      {
        current = (*inputPos)[j];
      }
      int bit = 1;
      // TODO: Enforce input restrictions.
      switch (current)
      {
      case '#':
        bit = 0;
        break;
      case '!':
        bit = 0;
        // Deliberate fall-through.
      case '@':
        // Bit is already set properly.
        mask->south = i;
        mask->west = j;
        mask->north = mask->height - 1 - mask->south;
        mask->east = mask->width - 1 - mask->west;
        break;
      case '.':
      default:
        // bit is already 1
        break;
      }
      if (bit == 1)
      {
        *intPos |= 0x1 << bitPos;
      }
      else
      {
        *intPos &= ~(0x1 << bitPos);
      }
      ++bitPos;
      if (bitPos == BITS_PER_INT)
      {
        bitPos = 0;
        ++intPos;
      }
    }
  }
  return result;
}

void cleanupPermissiveMask(permissiveMaskT * mask)
{
  delete [] mask->mask;
  mask->mask = NULL;
}

permissiveErrorT savePermissiveMask(permissiveMaskT * mask,
                                    char const * fileName)
{
  permissiveErrorT result = PERMISSIVE_NO_FAILURE;
  std::ofstream file(fileName, std::ios::out | std::ios::trunc);
  if (!file)
  {
    result = PERMISSIVE_FAILED_TO_OPEN_FILE;
  }
  else
  {
    for (int y = - mask->south; y <= mask->north && file; ++y)
    {
      for (int x = - mask->west; x <= mask->east && file; ++x)
      {
        if (x == 0 && y == 0)
        {
          if (doesPermissiveVisit(mask, x, y))
          {
            file << '@';
          }
          else
          {
            file << '!';
          }
        }
        else
        {
          if (doesPermissiveVisit(mask, x, y))
          {
            file << '.';
          }
          else
          {
            file << '#';
          }
        }
      }
      file << '\n';
    }
    if (!file)
    {
      result = PERMISSIVE_SAVE_WRITE_FAILED;
    }
  }
  return result;
}

void setPermissiveVisit(permissiveMaskT * mask, int x, int y)
{
  if (mask->mask != NULL)
  {
    int index = GET_INT(x + mask->west, y + mask->south);
    int shift = GET_BIT(x + mask->west, y + mask->south);
    mask->mask[index] |= 0x1 << shift;
  }
}

void clearPermissiveVisit(permissiveMaskT * mask, int x, int y)
{
  if (mask->mask != NULL)
  {
    int index = GET_INT(x + mask->west, y + mask->south);
    int shift = GET_BIT(x + mask->west, y + mask->south);
    mask->mask[index] &= ~(0x1 << shift);
  }
}

int doesPermissiveVisit(permissiveMaskT * mask, int x, int y)
{
  if (mask->mask == NULL)
  {
    return 1;
  }
  else
  {
    int index = GET_INT(x + mask->west, y + mask->south);
    int shift = GET_BIT(x + mask->west, y + mask->south);
    return (mask->mask[index] >> shift) & 0x1;
  }
}
