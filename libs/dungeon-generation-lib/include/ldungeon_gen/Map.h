/*
 * Map.h:
 *  An abstract map format, where each square has a set of labels and a content number.
 *  16 bits are set out for labels. 8 are predefined, 8 are user defined.
 *  Provides efficient & flexible mechanisms for querying & operating on large areas.
 */

#ifndef LDUNGEON_MAP_H_
#define LDUNGEON_MAP_H_

#include <lcommon/geometry.h>
#include <lcommon/Grid.h>
#include <lcommon/int_types.h>
#include <lcommon/mtwist.h>
#include <lcommon/smartptr.h>

#include <luawrap/LuaValue.h>

class SerializeBuffer;

namespace ldungeon_gen {

	const uint16_t FLAG_SOLID = 1 << 0;
	const uint16_t FLAG_PERIMETER = 1 << 1;
	const uint16_t FLAG_TUNNEL = 1 << 2;
	const uint16_t FLAG_HAS_OBJECT = 1 << 3;
	const uint16_t FLAG_NEAR_PORTAL = 1 << 4;
	const uint16_t FLAG_SEETHROUGH = 1 << 5;
	/* For future use */
	const uint16_t FLAG_RESERVED1 = 1 << 6;
	const uint16_t FLAG_RESERVED2 = 1 << 7;

	/* Identifies squares to act on based on their fields.
	 */
	struct Selector {
		uint16_t must_be_on_bits;
		uint16_t must_be_off_bits;
		uint16_t must_be_content;
		uint16_t must_be_group;
		bool use_must_be_content;

		Selector(uint16_t must_be_on_bits, uint16_t must_be_off_bits,
				uint16_t must_be_content) :
						must_be_on_bits(must_be_on_bits),
						must_be_off_bits(must_be_off_bits),
						must_be_content(must_be_content),
						must_be_group(-1),
						use_must_be_content(true) {
		}

		Selector(uint16_t must_be_on_bits = 0, uint16_t must_be_off_bits = 0) :
						must_be_on_bits(must_be_on_bits),
						must_be_off_bits(must_be_off_bits),
						must_be_content(0),
						must_be_group(-1),
						use_must_be_content(false) {
		}

		bool operator==(const Selector& o) {
			return must_be_on_bits == o.must_be_on_bits
					&& must_be_off_bits == o.must_be_off_bits
					&& must_be_content == o.must_be_content
					&& must_be_group == o.must_be_group
					&& use_must_be_content == o.use_must_be_content;
		}
	};

	/* Operates on the labels and content of a square.
	 * The 'most obvious' use case is clearing out an area.
	 */
	struct Operator {
		uint16_t turn_on_bits;
		uint16_t turn_off_bits;
		uint16_t flip_bits;
		uint16_t content_value;
		uint16_t group_value;

		Operator(uint16_t turn_on_bits, uint16_t turn_off_bits, uint16_t flip_bits,
				uint16_t content_value) :
						turn_on_bits(turn_on_bits),
						turn_off_bits(turn_off_bits),
						flip_bits(flip_bits),
						content_value(content_value),
                                                group_value(-1) {
		}

		Operator(uint16_t turn_on_bits = 0, uint16_t turn_off_bits = 0, uint16_t flip_bits = 0) :
						turn_on_bits(turn_on_bits),
						turn_off_bits(turn_off_bits),
						flip_bits(flip_bits),
						content_value(-1),
                                                group_value(-1) {
		}
		bool operator==(const Operator& o) {
			return turn_on_bits == o.turn_on_bits
					&& turn_off_bits == o.turn_off_bits
					&& flip_bits == o.flip_bits
					&& content_value == o.content_value
					&& group_value == o.group_value;
		}
	};

	struct ConditionalOperator {
		Selector selector;
		Operator oper;
		ConditionalOperator() {
		}
		ConditionalOperator(Selector selector, Operator oper) :
						selector(selector),
						oper(oper) {
		}
		bool operator==(const ConditionalOperator& o) {
			return selector == o.selector && oper == o.oper;
		}
	};

	struct Square {
		/* Bits 8 through 16 are used for 'user-defined flags.
		 * These are labeled in the lua-side. */
		uint16_t flags;
		/* Content number, based on flags */
		uint16_t content;
		/* Group this square belongs to */
		uint16_t group;

		Square(uint16_t flags = 0, uint16_t content = 0, uint16_t group = 0) :
						flags(flags),
						content(content),
						group(group) {
		}
		inline bool matches(Selector selector) const {
			return ((flags & selector.must_be_on_bits)
					== selector.must_be_on_bits)
					&& ((flags & ~selector.must_be_off_bits)
							== flags)
					&& (!selector.use_must_be_content
							|| content == selector.must_be_content);
		}
		inline void apply(Operator oper) {
			/* By turning off first, we can wipe all flags and set new ones */
			flags &= ~oper.turn_off_bits;
			flags |= oper.turn_on_bits;
			flags ^= oper.flip_bits;
			if (oper.content_value != -1) {
				content = oper.content_value;
			}
			if (oper.group_value != -1) {
				group = oper.group_value;
                        }
		}
		inline void apply(ConditionalOperator oper) {
			if (matches(oper.selector)) {
				apply(oper.oper);
			}
		}
		bool matches_flags(uint16_t flags) const {
			return this->flags & flags;
		}
	};

	struct Group {
		/* Group ID relationship */
		int group_id, parent_group_id;
		std::vector<int> child_group_ids;
		/* Bounding box of area */
		BBox group_area;

		Group(int group_id, int parent_group_id, const BBox& group_area) :
						group_id(group_id),
						parent_group_id(parent_group_id),
						group_area(group_area) {
		}

		void serialize(SerializeBuffer& serializer) const;
		void deserialize(SerializeBuffer& serializer);
	};

	const int ROOT_GROUP_ID = 0;
	typedef int group_t;

	/* Inheritance used for Grid methods */
	class Map: public Grid<Square> {
	public:
		Map(const Size& size = Size(), const Square& fill_value = Square());
		std::vector<Group> groups;
		LuaValue luafields;

		group_t make_group(const BBox& area, int parent_group_id);

		/* For testing purposes with serialization */
		bool operator==(const Map& map) const;
		bool operator!=(const Map& map) const {
			return !(*this == map);
		}

		void serialize(SerializeBuffer& serializer) const;
		void deserialize(SerializeBuffer& serializer);
		void clear() {
		    resize(Size(0,0));
		    groups.clear();
		    luafields.clear();
		}
	};

	typedef smartptr<Map> MapPtr;
}

#endif /* MAP_H_ */
