/*
 * Map.cpp:
 *  An abstract map format, where each square has a set of labels and a content number.
 *  16 bits are set out for labels. 8 are predefined, 8 are user defined.
 *  Provides efficient & flexible mechanisms for querying & operating on large areas.
 */

#include <lcommon/geometry.h>
#include <lcommon/SerializeBuffer.h>

#include <cstdio>

#include "Map.h"

namespace ldungeon_gen {

	Map::Map(const Size& size, const Square& fill_value) :
					Grid<Square>(size, fill_value) {
		/* Root parent group, allows group 0 to always be valid
		 * Things that apply to group 0 always apply to everything */
		groups.push_back(Group(0, -1, BBox(Pos(0,0), size)));
	}

	group_t Map::make_group(const BBox& area, int parent_group_id) {
		int group_id = groups.size();

		groups.at(parent_group_id).child_group_ids.push_back(group_id);
		groups.push_back(Group(group_id, parent_group_id, area));

		/* Label the rectangular area as our group */
		int rowstart = area.y1 * width();
		for (int y = area.y1; y < area.y2; y++) {
			int idx = rowstart + area.x1;
			for (int x = area.x1; x < area.x2; x++) {
				raw_get(idx).group = group_id;
				idx++;
			}
			rowstart += width();
		}

		return group_id;
	}

	/* For testing purposes with serialization */
	bool Map::operator ==(const Map& map) const {
		if (size() != map.size()) {
			return false;
		}

		if (groups.size() != map.groups.size()) {
			return false;
		}

		for (int i = 0; i < groups.size(); i++) {
			const Group& g1 = groups[i];
			const Group& g2 = map.groups[i];
			if (g1.child_group_ids != g2.child_group_ids
					|| g1.group_area != g2.group_area
					|| g1.group_id != g2.group_id
					|| g1.parent_group_id != g2.parent_group_id) {
				return false;
			}
		}

		Pos xy;
		for (xy.y = 0; xy.y < map.height(); xy.y++) {
			for (xy.x = 0; xy.x < map.width(); xy.x++) {
				if (memcmp(&(*this)[xy], &map[xy], sizeof(Square)) != 0) {
					return false;
				}
			}
		}

		return true;
	}

	void Group::serialize(SerializeBuffer& serializer) const {
		serializer.write(group_id);
		serializer.write(parent_group_id);
		serializer.write_container(child_group_ids);
		serializer.write(group_area);
	}

	void Group::deserialize(SerializeBuffer& serializer) {
		serializer.read(group_id);
		serializer.read(parent_group_id);
		serializer.read_container(child_group_ids);
		serializer.read(group_area);
	}

	void Map::serialize(SerializeBuffer& serializer) const {
		serializer.write(size());
		serializer.write_container(_internal_vector());
		serializer.write_int(groups.size());
		serializer.write(group_counter);
		serializer.write(max_group_counter);
		for (int i = 0; i < groups.size(); i++) {
			groups[i].serialize(serializer);
		}
	}

	void Map::deserialize(SerializeBuffer& serializer) {
		Size new_size;
		serializer.read(new_size);
		serializer.read_container(_internal_vector());
		serializer.read(group_counter);
		serializer.read(max_group_counter);
		resize(new_size);

		int group_size;
		serializer.read_int(group_size);
		groups.resize(group_size, Group(0, 0, BBox()));

		for (int i = 0; i < groups.size(); i++) {
			groups[i].deserialize(serializer);
		}
	}
}
