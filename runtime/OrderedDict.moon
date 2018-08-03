type = type  -- cache

_keys = (t) -> return for k,v in pairs(t)
    assert type(k) == "string" or type(k) == "number"
    k

_cmp_mixed_type = (a, b) ->
    ta, tb = type(a), type(b)
    if ta == tb
        return a < b
    else
        return ta < tb

TOMBSTONE = {}

_meta = {
    __index: (k) =>
        return @__vals[k]
    __newindex: (k, v) =>
        assert k ~= nil
        tmbs = @__tombstone_idxs
        if v == nil -- member deletion, O(N), use tombstone 
            for i, key in ipairs(@__keys)
                if key == k
                    @__keys[i] = TOMBSTONE
                    append tmbs, i
                    break
        elseif @__vals[k] == nil -- member creation, O(1)
            last_tmb = tmbs[#tmbs]
            if tmbs[#tmbs] ~= nil
                @__keys[last_tmb] = k
                tmbs[#tmbs] = nil
            else
                append @__keys, k
        @__vals[k] = v
        return
    __pairs: () =>
        i = 1
        return () ->
            -- Tombstone allows for in-place modification
            key = TOMBSTONE
            while key == TOMBSTONE
                key = @__keys[i]
                i += 1
            if key ~= nil
                return key, @__vals[key]
            else
                return
}

_init = (dict) =>
    if dict ~= nil
        assert not getmetatable(dict)
        @__keys = _keys(dict)
        table.sort(@__keys, _cmp_mixed_type)
        @__vals = dict
    else
        @__keys = {} -- Key order list
        @__vals = {} -- Dictionary
    @__tombstone_idxs = {}
    return setmetatable(@, _meta)

-- OrderedDict:
return {
    OrderedDict: (dict) -> _init({}, dict)
    :TOMBSTONE
}
