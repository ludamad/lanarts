/*
 * Grid.h:
 *  Represents a rectangular grid. Holds any type.
 */

#ifndef GRID_H_
#define GRID_H_

#include <vector>
#include <lcommon/geometry.h>

#include "lanarts_defines.h"

template <typename V>
class Grid {
private:

	typedef typename std::vector<V>::reference Ref;
	typedef typename std::vector<V>::const_reference ConstRef;

public:

	Grid(const Size& size = Size(), const V& fill_value = V())
		: _contents(size.w * size.h, fill_value) {
		_size = size;
	}

	/**
	 * Get the element at 'xy'.
	 */
	Ref operator[](const Pos& xy) {
		LANARTS_ASSERT(xy.x >= 0 && xy.x < _size.w);
		LANARTS_ASSERT(xy.y >= 0 && xy.y < _size.h);
		return _contents[xy.y * _size.w + xy.x];
	}

	/**
	 * Get the element at 'xy' (but unmodifiable).
	 */
	ConstRef operator[](const Pos& xy) const {
		LANARTS_ASSERT(xy.x >= 0 && xy.x < _size.w);
		LANARTS_ASSERT(xy.y >= 0 && xy.y < _size.h);
		return _contents[xy.y * _size.w + xy.x];
	}

	/**
	 * Access the underlying vector directly.
	 */
	Ref raw_get(int idx) {
		return _contents[idx];
	}

	/**
	 * Access the underlying vector directly (but unmodifiable).
	 */
	ConstRef raw_get(int idx) const {
		return _contents[idx];
	}

	/**
	 * Resizes to 'size'.
	 * NOTE: Contents will be invalid after, and should be reinitialized.
	 *
	 * A special case is if the grid was 0x0 prior, it will then be initialized
	 * as if it was 'cleared'. This makes it appropriate for constructors.
	 */
	void resize(const Size& size) {
		_size = size;
		_contents.resize(size.w * size.h);
	}

	/**
	 * Fills contents, by default fills with default constructor for V.
	 */
	void fill(const V& fill_value = V()) {
		typename std::vector<V>::iterator iter = _contents.begin();
		typename std::vector<V>::iterator end = _contents.end();

		while (iter != end) {
			*iter = fill_value;
			++iter;
		}
	}

	Size size() const {
		return _size;
	}

	V* begin() {
		return &_contents[0];
	}

	V* end() {
		return begin() + _size.w * _size.h;
	}

	/* Use only for serialization purposes!
	 * Subject to change. */
	std::vector<V>& _internal_vector() {
		return _contents;
	}

private:
	Size _size;
	std::vector<V> _contents;
};

#endif /* GRID_H_ */
