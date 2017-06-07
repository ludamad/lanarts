M = {}

_node = (handler_map) ->
    handlers = {}
    for k,v in pairs handler_map
        append handlers, v(k)
    local ret
    ret = setmetatable {
    }, {
        __call: (args) =>
            obj = {}
            for handler in *handlers
                handler(obj, args)
            for k,v in pairs(args)
                if obj[k] == nil
                    error("Unexpected argument #{k}")
            return setmetatable(obj, ret)
    }
    return ret

typeof = (obj) ->
    t = type(obj)
    if t == 'table'
        return rawget(obj, '_type') or t
    return t

from_set = (set) -> (name) -> (obj, args) ->
    val = args[name] 
    if val == nil
        error("Required argument #{name}")
    if not set[val]
        error("Invalid argument #{name} = #{val}")
    obj[name] = args[name]

typecheck = (type) -> (name) -> (obj, args) ->
    val = args[name] 
    if val == nil
        error("Required argument #{name}")
    if typeof(val) ~= type
        error("Invalid argument type #{name} = #{val}")
    obj[name] = args[name]

_CONNECTION_SCHEMES = {
    direct: true
}

M.Area = _node {
    name: typecheck("string")
    subareas: typecheck("table") -- TODO
    connection_scheme: from_set(_CONNECTION_SCHEMES)
}

M.Polygon = _node {
    name: typecheck("string")
    points: typecheck("table")

}

for k, v in pairs(M)
    v.name = k

return M
