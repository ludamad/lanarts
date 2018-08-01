local _keys
_keys = function(t)
  return (function()
    local _accum_0 = { }
    local _len_0 = 1
    for k, v in pairs(t) do
      assert(type(k) == "string" or type(k) == "number")
      local _value_0 = k
      _accum_0[_len_0] = _value_0
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()
end
local _meta = {
  __index = function(self, k)
    return self.vals[k]
  end,
  __newindex = function(self, k, v)
    assert(k ~= nil)
    if v == nil then
      table.remove_occurrences(self.keys, k)
    elseif self.vals[k] == nil then
      append(self.keys, v)
    end
    self.vals[k] = v
  end,
  __pairs = function(self)
    local i = 1
    assert(self.keys, self.vals)
    return function()
      local key = self.keys[i]
      i = i + 1
      if key ~= nil then
        return key, self.vals[key]
      else
        return 
      end
    end
  end
}
local _init
_init = function(self, dict)
  if dict ~= nil then
    assert(not getmetatable(dict))
    self.keys = _keys(dict)
    table.sort(self.keys)
    self.vals = dict
  else
    self.keys = { }
    self.vals = { }
  end
  return setmetatable(self, _meta)
end
return function(dict)
  return _init({ }, dict)
end
