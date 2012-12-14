/*
 * image_tests.cpp:
 *  Test ldraw::Image properties
 */

#include <common/unittest.h>

#include "Image.h"

static void ldraw_image_test() {
	using namespace ldraw;
	Image img;
	img.draw_region() = BBoxF(0,0,100,100);
	UNIT_TEST_ASSERT(img.draw_region() == BBoxF(0,0,100,100));
	UNIT_TEST_ASSERT(img.animation_duration() == 0);
	UNIT_TEST_ASSERT(!img.is_animated());
}

void ldraw_image_tests() {
	UNIT_TEST(ldraw_image_test);
}
