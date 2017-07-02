M = nilprotect {}

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

optional = (type) -> (name) -> (obj, args) ->
    val = args[name] 
    if val == nil
        return
    if typeof(val) ~= type
        error("Invalid argument type #{name} = #{val}")
    obj[name] = args[name]

_CONNECTION_SCHEMES = {
    none: true
    direct: true
    direct_light: true
    minimum_spanning_arc_and_line: true
    random_tunnels: true
}

_SPREAD_SCHEMES = {
    random: true
    box2d: true
    rvo_spread: true
    rvo_center: true
    rvo_ring: true
    box2d_solid_center: true
}

M.Spread = _node {
    name: optional "string"
    regions: typecheck "table"
    connection_scheme: from_set(_CONNECTION_SCHEMES)
    spread_scheme: from_set(_SPREAD_SCHEMES)
    post_compile: optional "string"
}

M.Shape = _node {
    name: optional "string"
    shape: typecheck "string"
    size: typecheck "table"
    post_compile: optional "string"
    --rotation: optional "string" -- TODO
}

for k, v in pairs(M)
    v.name = k

return M
