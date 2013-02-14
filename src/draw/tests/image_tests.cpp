/*
 * image_tests.cpp:
 *  Test ldraw::Image properties
 */

#include <lcommon/unittest.h>

#include "Image.h"

SUITE(ldraw_image_tests) {
	TEST(ldraw_image_test) {
		using namespace ldraw;
		Image img;
		img.draw_region() = BBoxF(0, 0, 100, 100);
		UNIT_TEST_ASSERT(img.draw_region() == BBoxF(0,0,100,100));
		UNIT_TEST_ASSERT(img.animation_duration() == 0);
		UNIT_TEST_ASSERT(!img.is_animated());
	}
}
