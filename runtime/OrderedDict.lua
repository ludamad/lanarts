local type = type
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
local _cmp_mixed_type
_cmp_mixed_type = function(a, b)
  local ta, tb = type(a), type(b)
  if ta == tb then
    return a < b
  else
    return ta < tb
  end
end
local TOMBSTONE = { }
local _meta = {
  __index = function(self, k)
    return self.__vals[k]
  end,
  __newindex = function(self, k, v)
    assert(k ~= nil)
    local tmbs = self.__tombstone_idxs
    if v == nil then
      for i, key in ipairs(self.__keys) do
        if key == k then
          self.__keys[i] = TOMBSTONE
          append(tmbs, TOMBSTONE)
          break
        end
      end
    elseif self.__vals[k] == nil then
      local last_tmb = tmbs[#tmbs]
      if tmbs[#tmbs] ~= nil then
        self.__keys[last_tmb] = k
        tmbs[#tmbs] = nil
      else
        append(self.__keys, k)
      end
    end
    self.__vals[k] = v
  end,
  __pairs = function(self)
    local i = 1
    return function()
      local key = TOMBSTONE
      while key == TOMBSTONE do
        key = self.__keys[i]
        i = i + 1
      end
      if key ~= nil then
        return key, self.__vals[key]
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
    self.__keys = _keys(dict)
    table.sort(self.__keys, _cmp_mixed_type)
    self.__vals = dict
  else
    self.__keys = { }
    self.__vals = { }
  end
  self.__tombstone_idxs = { }
  return setmetatable(self, _meta)
end
return {
  OrderedDict = function(dict)
    return _init({ }, dict)
  end,
  TOMBSTONE = TOMBSTONE
}
