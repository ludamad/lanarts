/*
 * math_util.h
 *  Defines utility functions for things such as vector normalization, rounding to powers of two, etc
 */

#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

#include "game_basic_structs.h"

int power_of_two(int input);
void direction_towards(const Pos& a, const Pos& b, float& rx, float& ry, float speed = 1);
int squish(int a, int b, int c);
float distance_between(const Pos& a, const Pos& b);
#endif /* MATH_UTIL_H_ */
