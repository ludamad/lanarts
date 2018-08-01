_keys = (t) -> return for k,v in pairs(t)
    assert type(k) == "string" or type(k) == "number"
    k

TOMBSTONE = {}

compact = () =>
    table.remove_occurrences(@__keys, TOMBSTONE)
    return

_meta = {
    __index: (k) =>
        if k == '__compact'
            return compact
        return @__vals[k]
    __newindex: (k, v) =>
        assert k ~= nil
        if v == nil -- member deletion, O(N), use tombstone 
            for i, key in ipairs(@__keys)
                if key == k
                    @__keys[i] = TOMBSTONE
        elseif @__vals[k] == nil -- member creation, O(1)
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
        table.sort(@__keys)
        @__vals = dict
    else
        @__keys = {} -- Key order list
        @__vals = {} -- Dictionary
    return setmetatable(@, _meta)

-- OrderedDict:
return {
    OrderedDict: (dict) -> _init({}, dict)
    :TOMBSTONE
}
