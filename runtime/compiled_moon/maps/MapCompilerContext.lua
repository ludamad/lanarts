local Map = require("core.Map")
local World = require("core.World")
local PendingFeature = newtype({
  init = function(self, label, placement_function, max_tries)
    if max_tries == nil then
      max_tries = 100
    end
    self.label, self.placement_function, self.max_tries = label, placement_function, max_tries
    return assert(self.label)
  end,
  _place = function(self, args)
    local compiler = args.compiler
    for i = 1, self.max_tries do
      local compiled_feature = self:placement_function(compiler)
      if compiled_feature ~= nil then
        self.feature = compiled_feature
        return true
      end
    end
    return false
  end,
  compile = function(self, args)
    return self:_place(args)
  end
})
local make_on_player_interact
make_on_player_interact = function(cc, target_portal)
  return function(_, user)
    local destination_map = cc:get({
      label = assert(target_portal.label)
    })
    assert(target_portal.feature, "Connected portal should have compiled!")
    return Map.transfer(user, destination_map, target_portal.feature.xy)
  end
end
local PendingPortal = newtype({
  parent = PendingFeature,
  connect = function(self, other_portal)
    self.other_portal = other_portal
    return nil
  end,
  compile = function(self, args)
    if not PendingFeature.compile(self, args) then
      return false
    end
    assert(self.feature, "Portal should have compiled")
    self.feature.on_player_interact = make_on_player_interact(args.context, self.other_portal)
    return true
  end
})
local MapCompilerContext = newtype({
  init = function(self)
    self.pending_maps = { }
    self.maps = { }
    self.pending_features = { }
  end,
  add_pending_feature = function(self, label, feature)
    append(self.pending_features[label], feature)
    return feature
  end,
  add_pending_portal = function(self, label, placement_function)
    local feature = PendingPortal.create(label, placement_function)
    return self:add_pending_feature(label, feature)
  end,
  register = function(self, label, C)
    assert((not self.pending_maps[label]), tostring(label) .. " already registered!")
    assert(C)
    self.pending_maps[label] = C
    self.pending_features[label] = { }
  end,
  get = function(self, args)
    if (type(args)) == "string" then
      args = {
        label = args
      }
    end
    local label = assert(args.label, "Must provide label to identify map to retrieve/generate!")
    if (type(label)) ~= "string" then
      label = label.label
    end
    if not self.pending_maps[label] then
      return assert(self.maps[label], "Map '" .. tostring(label) .. "' not registered!")
    end
    log_info("Generating map '" .. tostring(label) .. "'!")
    local C = self.pending_maps[label]
    if type(C) == 'function' then
      args.label = label
      args.pending_feature = self.pending_features[label]
      self.maps[label] = C(self, args)
    else
      local compiler = C.create({
        label = label,
        rng = require('mtwist').create(random(0, 2 ^ 30)),
        spawn_players = args.spawn_players
      })
      if not compiler:prepare() then
        error("Map generation completely failed!!")
      end
      args.context = self
      args.compiler = compiler
      compiler:generate(args)
      local _list_0 = self.pending_features[label]
      for _index_0 = 1, #_list_0 do
        local pending_feature = _list_0[_index_0]
        pending_feature:compile(args)
      end
      self.maps[label] = compiler:compile(args)
    end
    self.pending_maps[label] = nil
    self.pending_features[label] = nil
    return self.maps[label]
  end
})
return {
  MapCompilerContext = MapCompilerContext,
  make_on_player_interact = make_on_player_interact
}
