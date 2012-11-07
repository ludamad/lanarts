/*
 * math.cpp:
 *  Math utility functions
 */

#include "math.h"

int power_of_two(int input) {
	int value = 1;

	while (value < input) {
		value <<= 1;
	}
	return value;
}
