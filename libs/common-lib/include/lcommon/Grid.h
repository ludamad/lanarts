/*
 * Grid.h:
 *  Represents a rectangular grid. Holds any type.
 */

#ifndef GRID_H_
#define GRID_H_

#include <memory>
#include <vector>
#include "geometry.h"
#include "lcommon_assert.h"

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
		LCOMMON_ASSERT(xy.x >= 0 && xy.x < _size.w);
		LCOMMON_ASSERT(xy.y >= 0 && xy.y < _size.h);
		return _contents[xy.y * _size.w + xy.x];
	}

	/**
	 * Get the element at 'xy' (but unmodifiable).
	 */
	ConstRef operator[](const Pos& xy) const {
		LCOMMON_ASSERT(xy.x >= 0 && xy.x < _size.w);
		LCOMMON_ASSERT(xy.y >= 0 && xy.y < _size.h);
		return _contents[xy.y * _size.w + xy.x];
	}

	/**
	 * Access the underlying vector directly.
	 */
	Ref raw_get(int idx) {
		LCOMMON_ASSERT(idx >= 0 && idx < _contents.size());
		return _contents[idx];
	}

	/**
	 * Access the underlying vector directly (but unmodifiable).
	 */
	ConstRef raw_get(int idx) const {
		LCOMMON_ASSERT(idx >= 0 && idx < _contents.size());
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

	Size& size() {
		return _size;
	}

	int width() const {
		return _size.w;
	}

	int height() const {
		return _size.h;
	}

	bool empty() const {
		return _size.w == 0 && _size.h == 0;
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
	const std::vector<V>& _internal_vector() const {
		return _contents;
	}

private:
	Size _size;
	std::vector<V> _contents;
};

template <typename V>
class GridSlice {
private:

	typedef typename std::vector<V>::reference Ref;
	typedef typename std::vector<V>::const_reference ConstRef;

public:

	GridSlice(BBox area, const std::shared_ptr<Grid<V>>& grid) : _area(area), _grid(grid) {
	}

	/**
	 * Get the element at 'xy'.
	 */
	Ref operator[](const Pos& xy) {
		return (*_grid)[xy + _area.left_top()];
	}

	/**
	 * Get the element at 'xy' (but unmodifiable).
	 */
	ConstRef operator[](const Pos& xy) const {
		return (*_grid)[xy + _area.left_top()];
	}

	/**
	 * Fills contents, by default fills with default constructor for V.
	 */
	void fill(const V& fill_value = V()) {
		FOR_EACH_BBOX(_area, x, y) {
			(*_grid)[{x,y}] = fill_value;
		}
	}

	Size size() const {
		return _area.size();
	}

	int width() const {
		return _area.width();
	}

	int height() const {
		return _area.height();
	}

	bool empty() const {
		return _area.empty();
	}

private:
	BBox _area;
	std::shared_ptr<Grid<V>> _grid;
};

#endif /* GRID_H_ */
