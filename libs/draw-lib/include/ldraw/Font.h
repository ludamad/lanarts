#ifndef LDRAW_FONT_H_
#define LDRAW_FONT_H_

#include <string>
#include <lcommon/smartptr.h>

struct Colour;
struct PosF;
struct SizeF;

namespace ldraw {

//private structure:
struct font_data;
struct DrawOptions;

class Font {
public:
	Font() {
	}
	Font(const char* filename, int height) {
		initialize(filename, height);
	}
	Font(const std::string& filename, int height) {
		initialize(filename, height);
	}

	void initialize(const char* filename, int height);
	void initialize(const std::string& filename, int height) {
		initialize(filename.c_str(), height);
	}

	// Standard font draw functions
	int draw(const DrawOptions& options, const PosF& position,
			const char* str) const;
	int draw(const DrawOptions& options, const PosF& position,
			const std::string& str) const {
		return draw(options, position, str.c_str());
	}

	// Wrap over a maxwidth:
	void draw_wrapped(const DrawOptions& options, const PosF& position,
			int maxwidth, const char* str) const;
	void draw_wrapped(const DrawOptions& options, const PosF& position,
			int maxwidth, const std::string& str) const {
		draw_wrapped(options, position, maxwidth, str.c_str());
	}

	//These operate similar to printf:
	int drawf(const DrawOptions& options, const PosF& position,
			const char* fmt, ...) const;
	void drawf_wrapped(const DrawOptions& options, const PosF& position,
			int maxwidth, const char* fmt, ...) const;

	// Return the size that would be drawn
	SizeF get_draw_size(const char* str, int maxwidth = -1) const;
	SizeF get_draw_size(const std::string& str, int maxwidth = -1) const;

	int height() const;

	void clear();
	bool empty() const {
		return _font.empty();
	}

private:
	mutable std::string _print_buffer;
	smartptr<font_data> _font;
};

}

#endif /* LDRAW_FONT_H_ */
