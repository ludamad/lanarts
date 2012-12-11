/*
 * animation_tests.cpp:
 *  Test ldraw::Animation properties
 */

#include <common/unittest.h>

#include "DrawableMock.h"

#include "../Image.h"
#include "../Animation.h"

static void ldraw_animation_properties_test() {
	using namespace ldraw;
	std::vector<Image> images;
	for (int i = 0; i < 3; i++) {
		images.push_back(Image());
	}
	Animation anim(images);

	UNIT_TEST_ASSERT(anim.animation_duration() == 3);
	UNIT_TEST_ASSERT(anim.is_animated());
}

//Test that it calls the correct mock
static void ldraw_animation_draw_test() {
	using namespace ldraw;
	using namespace ldraw::test;

	Animation anim;

	std::string called_string;
	anim.add(Drawable(new DrawableMock("0", &called_string)));
	anim.add(Drawable(new DrawableMock("1", &called_string)));
	anim.add(Drawable(new DrawableMock("2", &called_string)));

	anim.draw(DrawOptions().frame(0), Posf());
	UNIT_TEST_ASSERT(called_string == "0");

	anim.draw(DrawOptions().frame(1), Posf());
	UNIT_TEST_ASSERT(called_string == "1");

	anim.draw(DrawOptions().frame(2), Posf());
	UNIT_TEST_ASSERT(called_string == "2");

	anim.draw(DrawOptions().frame(3), Posf());
	UNIT_TEST_ASSERT(called_string == "0");
}

void ldraw_animation_tests() {
	UNIT_TEST(ldraw_animation_properties_test);
	UNIT_TEST(ldraw_animation_draw_test);

}


