/*
 * shapefill.h:
 *  Fill portions of a generated level with a shape, affecting arbitrary
 *  properties
 */

#ifndef SHAPEFILL_H_
#define SHAPEFILL_H_b

#include "generated_tile.h"
#include "GeneratedLevel.h"

/* Utility functions for creating square masks*/
Sqr empty_mask();

struct FillOption {
	Sqr field_values;
	Sqr field_values_ANDmask;
	bool use_filter;
	Sqr filter_values;
	Sqr filter_values_ANDmask;

	FillOption(const Sqr& value, const Sqr& value_mask, bool use_filter = false,
			const Sqr& filter = Sqr(), const Sqr& filter_mask = Sqr()) :
			field_values(value & (value_mask)), field_values_ANDmask(
					~value_mask), use_filter(use_filter), filter_values(
					filter & (~filter_mask)), filter_values_ANDmask(filter_mask) {
	}

	inline void fill_sqr(Sqr& sqr) const {
		Sqr andsqr = sqr & field_values_ANDmask;
		if (!use_filter || filter_values == (andsqr & filter_values_ANDmask)) {
			sqr = andsqr | field_values;
		}
	}
};

void level_rectangle_fill(GeneratedLevel& level, const BBox& region,
		const FillOption& fill);
void level_rectangle_outline(GeneratedLevel& level, const BBox& region,
		const FillOption& fill);
void level_oval_fill(GeneratedLevel& level, const BBox& region,
		const FillOption& fill);

typedef void (*shapefill_callback)(GeneratedLevel& level, const BBox& region,
		const FillOption& fill);

void set_passable(GeneratedLevel& level, const BBox& region,
		shapefill_callback shapefill, bool passable);
void set_passable_if(GeneratedLevel& level, const BBox& region,
		shapefill_callback shapefill, bool passable, const Sqr& filter_mask,
		const Sqr& filter);

#endif /* SHAPEFILL_H_ */
