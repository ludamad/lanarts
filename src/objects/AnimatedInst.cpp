/*
 * AnimatedInst.cpp:
 *  Represents an object that displays on the screen for a fixed amount of time before removing itself
 */

#include <ldraw/colour_constants.h>
#include <ldraw/DrawOptions.h>

#include <typeinfo>
#include <draw/fonts.h>

#include "draw/draw_sprite.h"
#include "draw/SpriteEntry.h"

#include "gamestate/GameState.h"
#include "gamestate/GameView.h"

#include "AnimatedInst.h"

AnimatedInst::~AnimatedInst() {
}

void AnimatedInst::step(GameState* gs) {
	x = round(rx += vx);
	y = round(ry += vy);
	if (timeleft != -1 && --timeleft < 0) {
		timeleft = 0;
		gs->remove_instance(this);
	}

}
void AnimatedInst::draw(GameState* gs) {
	GameView& view = gs->view();
	if (sprite > -1) {
		ldraw::Drawable& spr = res::sprite(sprite);

		SizeF size = spr.size();
		int xx = x - size.w / 2, yy = y - size.h / 2;

		if (!view.within_view(xx, yy, size.w, size.h))
			return;
		if (!gs->object_visible_test(this))
			return;

		Colour alphacol(255, 255, 255, 255 * timeleft / animatetime);

		float frame = animateframe != -1 ? animateframe : gs->frame();

		draw_sprite(view, sprite, xx, yy, orientx, orienty, frame,
				alphacol);

	}
	Colour wd(255 - textcol.r, 255 - textcol.g, 255 - textcol.b);
	if (text.size() > 0) {
		Colour alphacol = textcol;
		if (timeleft > -1) {
			int fade = 100 * timeleft / animatetime;
			alphacol = Colour(textcol.r + fade * wd.r / 100,
					textcol.g + fade * wd.g / 100,
					textcol.b + fade * wd.b / 100, 255 - fade);
		}

		ldraw::Font _font = font ? *font : res::font_primary();
		auto draw_options = ldraw::DrawOptions(should_center_font ? ldraw::CENTER : ldraw::LEFT_TOP, alphacol);
		_font.draw(draw_options, on_screen(gs, ipos()), text);
	}
}

void AnimatedInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(AnimatedInst*)inst = *this;
}

AnimatedInst* AnimatedInst::clone() const {
	return new AnimatedInst(*this);
}

void AnimatedInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::serialize(gs, serializer);
	SERIALIZE_POD_REGION(serializer, this, textcol, animatetime);
	serializer.write(text);
}

void AnimatedInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::deserialize(gs, serializer);
	DESERIALIZE_POD_REGION(serializer, this, textcol, animatetime);
	serializer.read(text);
}

