#ifndef RESOURCEDATASET_H_
#define RESOURCEDATASET_H_

#include <vector>
#include <string>
#include <cstring>
#include <luawrap/LuaValue.h>
#include "data/hashmap.h"

/*
 * ResourceDataSet:
 *   Represents a data set in the game.
 *   raw_data stores the unparsed Lua object used to define this data.
 *   list stores a list of the resource objects, from which they derive their ID.
 *   map_to_id stores a hash map of the resource object IDs, used for fast lookups into the list from their name.
 *
 *   For efficiency, the map_to_id object uses a specialized hashmap that does not require conversion to 'string'
 *   (like std::map or std::unordered_map), as this is by far the most common case.
 *
 *   NOT designed for deleting elements; only clearing.
 */
template <class R>
class ResourceDataSet {
public:
    enum class id_t {NONE = -1};

    ResourceDataSet() {
        map_to_id = hashmap_new();

    }
    ~ResourceDataSet() {
        hashmap_free(map_to_id);
        for (char* key : allocated_keys) {
            delete[] key;
        }
    }

    // Clears the data set. Can be called multiple times.
    void init(lua_State* L) {
        hashmap_free(map_to_id);
        for (char* key : allocated_keys) {
            delete[] key;
        }
        allocated_keys.clear();
        map_to_id = hashmap_new();
        list.clear();

        raw_data.init(L);
        raw_data.newtable();
    }

    R& new_entry(const std::string& name) {
        return new_entry(name.c_str());
    }
    R& new_entry(const char* name) {
        if (_get_id(name) != id_t::NONE) {
            throw std::runtime_error("Entry '" + std::string(name) + "' already exists!");
        }
        id_t next_id = static_cast<id_t>(list.size());
        _put_id(name, next_id);
        list.resize(list.size() + 1);
        return list[next_id];
    }
    R& get(const char* name) {
        id_t id = _get_id(name);
        if (id == id_t::NONE) {
            throw std::runtime_error("Entry '" + std::string(name) + "' does not exist!");
        }
        return get(id);
    }
    R& get(id_t id) {
        int id_i = static_cast<int>(id);
        return list.at(id);
    }
    size_t size() {
        return list.size();
    }

    void set_raw_data(const char* name, LuaValue value) {
        raw_data[name] = value;
    }

private:
    char* strclone(const char* name) {
        int len = strlen(name);
        char* cpy = new char[len + 1];
        for (int i = 0; i < len; i++) {
            cpy[i] = name[i];
        }
        cpy[len] = 0;
        allocated_keys.push_back(cpy);
        return cpy;
    }
    // Only call after confirming _get_id returns NONE
    void _put_id(const char* name, id_t id) {
        char* name_cpy = strclone(name);
        hashmap_put(map_to_id, name_cpy, static_cast<any_t>(id));
    }
    id_t _get_id(const char* name) {
        any_t result = nullptr;
        if (hashmap_get(map_to_id, (char*)name, &result) == MAP_MISSING) {
            return id_t::NONE;
        }
        return static_cast<id_t>(result);
    }

    std::vector<R> list;

    // Maps resource names to object values
    map_t map_to_id = nullptr;
    std::vector<char*> allocated_keys;
    LuaValue raw_data;
};

#endif
