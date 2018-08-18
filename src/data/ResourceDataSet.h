#ifndef RESOURCEDATASET_H_
#define RESOURCEDATASET_H_

#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <luawrap/luawrap.h>
#include "data/hashmap.h"
#include "lua_util.h"

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
    void clear() {
        hashmap_free(map_to_id);
        for (char* key : allocated_keys) {
            delete[] key;
        }
        allocated_keys.clear();
        map_to_id = nullptr;
        list.clear();
        raw_data = LuaValue();
    }
    void init(lua_State* L) {
        clear();
        map_to_id = hashmap_new();
        raw_data.init(L);
        raw_data.newtable();
    }

    R& new_entry(const std::string& name, const R& entry, LuaValue value) {
        set_raw_data(name, value);
        return new_entry(name.c_str(), entry);
    }
    R& new_entry(const std::string& name, const R& entry) {
        return new_entry(name.c_str(), entry);
    }
    R& new_entry(const char* name, const R& entry) {
        if (_get_id(name) != id_t::NONE) {
            throw std::runtime_error(("Entry '" + std::string(name) + "' already exists!").c_str());
        }
        id_t next_id = static_cast<id_t>(list.size());
        _put_id(name, next_id);
        list.push_back(entry);
        return list.back();
    }
    R& get(const std::string& name) {
        return get(name.c_str());
    }
    R& get(const char* name) {
        id_t id = _get_id(name);
        if (id == id_t::NONE) {
            throw std::runtime_error(("Entry '" + std::string(name) + "' does not exist!").c_str());
        }
        return get((int)id);
    }
    id_t get_id(const char* name) {
        return _get_id(name);
    }
    id_t get_id(const std::string& name) {
        return _get_id(name.c_str());
    }
    R& get(int id) {
        return list.at(id);
    }
    size_t size() {
        return list.size();
    }

    LuaValue& get_raw_data() {
        return raw_data;
    }

    template <class ReturnType, class... Args>
    ReturnType call_lua(const std::string& name, const char* method, ReturnType _default, Args... args) {
        lua_State* L = raw_data.luastate();
        raw_data[name.c_str()][method].push();
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            return _default;
        }
        return luawrap::call<ReturnType>(L, args...);
    }

    void set_raw_data(const std::string& name, LuaValue value) {
        raw_data[name.c_str()] = value;
    }

    void set_raw_data(const char* name, LuaValue value) {
        raw_data[name] = value;
    }

    void set_on_miss_fallback(LuaValue value) {
        on_miss = value;
    }
    void ensure_exists(const char* name) {
        any_t result = nullptr;
        auto lookup_status = hashmap_get(map_to_id, (char*)name, &result);
        if (lookup_status == MAP_MISSING && !on_miss.empty()) {
            lcall(on_miss, name);
            // Retry after giving on_miss a chance to generate an entry for 'name':
            lookup_status = hashmap_get(map_to_id, (char*)name, &result);
//            LANARTS_ASSERT(lookup_status != MAP_MISSING);
        }
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
        hashmap_put(map_to_id, name_cpy, (any_t)(long long)id);
    }
    id_t _get_id(const char* name) {
        any_t result = nullptr;

        auto lookup_status = hashmap_get(map_to_id, (char*)name, &result);
        if (lookup_status == MAP_MISSING) {
            return id_t::NONE;
        }
        return static_cast<id_t>((long long)result);
    }

    std::vector<R> list;

    // Maps resource names to object values
    map_t map_to_id = nullptr;
    std::vector<char*> allocated_keys;
    LuaValue raw_data;
    LuaValue on_miss;
};

#endif
