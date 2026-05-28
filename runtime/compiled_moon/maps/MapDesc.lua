local NewMaps = require("maps.NewMaps")
local SourceMap = require("core.SourceMap")
local Tilesets = require("tiles.Tilesets")
local from_bbox
from_bbox = require("maps.MapRegion").from_bbox
local DEFAULT_PADDING = 10
local MAX_MAP_TRIES = 100
local MapDesc = newtype({
  init = function(self, map_args)
    self.map_args = map_args
    self.parent = false
    self.children = self.map_args.children
    local _list_0 = self.children
    for _index_0 = 1, #_list_0 do
      local child = _list_0[_index_0]
      if child.tag == "" then
        child.tag = "root"
      end
    end
    self.map_label = self.map_args.map_label or "Dungeon"
    self.padding = self.map_args.padding or DEFAULT_PADDING
    self.map_args.children = nil
    self.map_args.padding = nil
    self.map_args.map_label = nil
    self.region_set = false
  end,
  linker = function(self, generate_hooks)
    if generate_hooks == nil then
      generate_hooks = { }
    end
    return require("maps.MapLink").MapLinker.create(self, generate_hooks)
  end,
  generate = function(self, back_links, forward_links, generate_hooks)
    if back_links == nil then
      back_links = { }
    end
    if forward_links == nil then
      forward_links = { }
    end
    if generate_hooks == nil then
      generate_hooks = { }
    end
    local map = self:compile(back_links, forward_links, generate_hooks)
    local game_map = NewMaps.generate_game_map(map)
    local _list_0 = map.post_game_map
    for _index_0 = 1, #_list_0 do
      local post_poned = _list_0[_index_0]
      post_poned(game_map)
    end
    return game_map
  end,
  compile = function(self, back_links, forward_links, generate_hooks)
    if back_links == nil then
      back_links = { }
    end
    if forward_links == nil then
      forward_links = { }
    end
    if generate_hooks == nil then
      generate_hooks = { }
    end
    return NewMaps.try_n_times(MAX_MAP_TRIES, function()
      return self:_compile(back_links, forward_links, generate_hooks)
    end)
  end,
  _compile = function(self, back_links, forward_links, generate_hooks)
    if back_links == nil then
      back_links = { }
    end
    if forward_links == nil then
      forward_links = { }
    end
    if generate_hooks == nil then
      generate_hooks = { }
    end
    self.back_links, self.forward_links, self.generate_hooks = back_links, forward_links, generate_hooks
    local map_args = table.merge({
      rng = NewMaps.new_rng(),
      size = {
        100,
        100
      },
      default_content = Tilesets.orc.wall,
      default_flags = {
        SourceMap.FLAG_SOLID
      },
      map_label = self.map_label,
      post_map = { }
    }, self.map_args)
    map_args.size = vector_add(map_args.size, {
      self.padding * 2,
      self.padding * 2
    })
    local map = NewMaps.source_map_create(map_args)
    map.post_map = { }
    local room_selector = {
      matches_all = SourceMap.FLAG_SOLID,
      matches_none = SourceMap.FLAG_PERIMETER
    }
    self.region_set = {
      map = map,
      regions = {
        from_bbox(self.padding, self.padding, map.size[1] - self.padding, map.size[2] - self.padding):with_selector(room_selector)
      }
    }
    local _list_0 = self.children
    for _index_0 = 1, #_list_0 do
      local child = _list_0[_index_0]
      child.parent = self
      child.desc = self
      if not child:compile() then
        return nil
      end
    end
    local _list_1 = map.post_map
    for _index_0 = 1, #_list_1 do
      local post_map = _list_1[_index_0]
      if not post_map(map) then
        return nil
      end
    end
    if not NewMaps.check_connection(map) then
      event_log("Failed connectivity check")
      return nil
    end
    return map
  end
})
local MapNode = newtype({
  init = function(self, args)
    self.children = args.children or { }
    self.place = args.place
    self.tag = args.tag or ""
  end,
  new_group = function(self)
    self.map.next_group = self.map.next_group + 1
    return self.map.next_group
  end,
  compile = function(self)
    self.region_set = self.parent.region_set
    self.group = self:new_group()
    if not self:place() then
      return false
    end
    local hook = self.desc.generate_hooks[self.tag]
    if hook and not hook(self) then
      return false
    end
    local _list_0 = self.children
    for _index_0 = 1, #_list_0 do
      local child = _list_0[_index_0]
      child.parent = self
      child.desc = self.desc
      if not child:compile() then
        return false
      end
    end
    return true
  end,
  chance = function(self, prob)
    return self.region_set.map.rng:chance(prob)
  end,
  apply = function(self, args)
    args.map = self.map
    local _list_0 = self.region_set.regions
    for _index_0 = 1, #_list_0 do
      local region = _list_0[_index_0]
      region:apply(args)
    end
  end,
  get = {
    map = function(self)
      return self.region_set.map
    end,
    rng = function(self)
      return self.region_set.map.rng
    end
  }
})
return nilprotect({
  MapNode = MapNode,
  MapDesc = MapDesc
})
