_keys = (t) -> for k,v in pairs(t)
    assert type(k) == "string" or type(k) == "number"
    k

_meta = {
    __index: (k) =>
        return @vals[k]
    __newindex: (k, v) =>
        assert k ~= nil
        if v == nil -- member deletion, O(N)
            table.remove_occurrences @keys, k
        elseif @vals[k] == nil -- member creation, O(1)
            append @keys, v
        @vals[k] = v
        return
    __pairs: () =>
        i = 1
        return () ->
            key = @keys[i]
            i += 1
            if key ~= nil
                return key, @vals[key]
            else
                return
}

_init = (dict) =>
    if dict ~= nil
        @keys = _keys(dict)
        @vals = dict
    else
        @keys = {} -- Key order list
        @vals = {} -- Dictionary
    return setmetatable(@, _meta)

-- OrderedDict:
return (dict) -> _init({}, dict)
