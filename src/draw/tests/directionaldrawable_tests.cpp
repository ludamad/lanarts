/*
 * directionaldrawable_tests.cpp:
 *  Test ldraw::DirectionalDrawable properties
 */

#include <lcommon/unittest.h>
#include <lcommon/math_util.h>

#include "DrawableMock.h"

#include "Image.h"
#include "DirectionalDrawable.h"

static void ldraw_directionaldrawable_properties_test() {
	using namespace ldraw;

	std::vector<Image> images;
	for (int i = 0; i < 3; i++) {
		images.push_back(Image());
	}

	DirectionalDrawable dd(images);
	UNIT_TEST_ASSERT(dd.animation_duration() == 0);
	UNIT_TEST_ASSERT(!dd.is_animated());
}

//Test that it calls the correct mock
static void ldraw_directionaldrawable_draw_test() {
	using namespace ldraw;
	using namespace ldraw::test;

	DirectionalDrawable dd;

	std::string called_string;
	dd.add(Drawable(new DrawableMock("0", &called_string)));
	dd.add(Drawable(new DrawableMock("1", &called_string)));
	dd.add(Drawable(new DrawableMock("2", &called_string)));

	float ONE_THIRD = FLOAT_PI*2/3;

	dd.draw(DrawOptions().angle(0*ONE_THIRD), Posf());
	UNIT_TEST_ASSERT(called_string == "0");
	dd.draw(DrawOptions().angle(1*ONE_THIRD), Posf());
	UNIT_TEST_ASSERT(called_string == "1");
	dd.draw(DrawOptions().angle(2*ONE_THIRD), Posf());
	UNIT_TEST_ASSERT(called_string == "2");
	dd.draw(DrawOptions().angle(3*ONE_THIRD), Posf());
	UNIT_TEST_ASSERT(called_string == "0");
}


void ldraw_directionaldrawable_tests() {
	UNIT_TEST(ldraw_directionaldrawable_properties_test);
	UNIT_TEST(ldraw_directionaldrawable_draw_test);

}
