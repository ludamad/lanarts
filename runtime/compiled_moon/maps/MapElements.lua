local M = nilprotect({ })
local _node
_node = function(handler_map)
  local handlers = { }
  for k, v in pairs(handler_map) do
    append(handlers, v(k))
  end
  local ret
  ret = setmetatable({ }, {
    __call = function(self, args)
      local obj = { }
      for _index_0 = 1, #handlers do
        local handler = handlers[_index_0]
        handler(obj, args)
      end
      for k, v in pairs(args) do
        if obj[k] == nil then
          error("Unexpected argument " .. tostring(k))
        end
      end
      return setmetatable(obj, ret)
    end
  })
  return ret
end
local typeof
typeof = function(obj)
  local t = type(obj)
  if t == 'table' then
    return rawget(obj, '_type') or t
  end
  return t
end
local from_set
from_set = function(set, allow_nesting)
  return function(name)
    return function(obj, args)
      local val = args[name]
      if allow_nesting and type(val) == "table" and set[val.type] then
        val = val.type
      end
      if val == nil then
        error("Required argument " .. tostring(name))
      end
      if type(val) ~= "function" then
        if not set[val] then
          error("Invalid argument " .. tostring(name) .. " = " .. tostring(val))
        end
      end
      obj[name] = args[name]
    end
  end
end
local typecheck
typecheck = function(type)
  return function(name)
    return function(obj, args)
      local val = args[name]
      if val == nil then
        error("Required argument " .. tostring(name))
      end
      if typeof(val) ~= type then
        error("Invalid argument type " .. tostring(name) .. " = " .. tostring(val))
      end
      obj[name] = args[name]
    end
  end
end
local _flatten
_flatten = function(t, accum)
  if accum == nil then
    accum = { }
  end
  if getmetatable(t) then
    append(accum, t)
  else
    for _index_0 = 1, #t do
      local elem = t[_index_0]
      _flatten(elem, accum)
    end
  end
  return accum
end
local flat_table
flat_table = function(name)
  return function(obj, args)
    obj[name] = _flatten(args[name])
  end
end
local any
any = function(name)
  return function(obj, args)
    obj[name] = args[name]
    return nil
  end
end
local optional
optional = function(type)
  return function(name)
    return function(obj, args)
      local val = args[name]
      if val == nil then
        return
      end
      if typeof(val) ~= type then
        error("Invalid argument type " .. tostring(name) .. " = " .. tostring(val))
      end
      obj[name] = args[name]
    end
  end
end
local _CONNECTION_SCHEMES = {
  none = true,
  direct = true,
  direct_light = true,
  minimum_spanning_arc_and_line = true,
  random_tunnels = true
}
local _SPREAD_SCHEMES = {
  random = true,
  box2d = true,
  rvo_spread = true,
  rvo_center = true,
  rvo_ring = true,
  box2d_solid_center = true
}
local _FILL_SCHEMES = { }
M.Spread = _node({
  name = optional("string"),
  regions = flat_table,
  connection_scheme = from_set(_CONNECTION_SCHEMES, true),
  spread_scheme = from_set(_SPREAD_SCHEMES, true),
  paint = optional("function"),
  place_objects = optional("function"),
  properties = optional("table")
})
M.Shape = _node({
  name = optional("string"),
  shape = any,
  size = typecheck("table"),
  embeds = optional("table"),
  paint = optional("function"),
  place_objects = optional("function"),
  properties = optional("table")
})
for k, v in pairs(M) do
  v.name = k
end
return M
