local SourceMap = require("core.SourceMap")
local MapUtils = require("maps.MapUtils")
local TileSets = require("tiles.Tilesets")
local Display = require("core.Display")
local M = nilprotect({ })
M.FLAG_ALTERNATE = SourceMap.FLAG_CUSTOM1
M.FLAG_INNER_PERIMETER = SourceMap.FLAG_CUSTOM2
M.FLAG_DOOR_CANDIDATE = SourceMap.FLAG_CUSTOM3
M.FLAG_OVERWORLD = SourceMap.FLAG_CUSTOM4
M.FLAG_ROOM = SourceMap.FLAG_CUSTOM5
M.FLAG_NO_ENEMY_SPAWN = SourceMap.FLAG_CUSTOM6
M.FLAG_NO_ITEM_SPAWN = SourceMap.FLAG_CUSTOM7
M.FLAG_TEMPORARY = M.FLAG_ROOM
M.FLAG_HAS_VAULT = SourceMap.FLAG_CUSTOM8
local FLAGS_HAS_CONTENT = M.FLAG_HAS_VAULT + SourceMap.FLAG_HAS_OBJECT
M._warning_skull = Display.image_load("features/sprites/warning.png")
M._rune_door_closed = Display.image_load("spr_doors/runed_door.png")
M._closed_door_crypt = Display.image_load("spr_doors/closed_door_crypt.png")
M._door_key1 = Display.image_load("spr_keys/door01.png")
M._door_key2 = Display.image_load("spr_keys/door02.png")
M._door_key3 = Display.image_load("spr_keys/door03.png")
M._door_magentite = Display.image_load("spr_keys/magentite_door.png")
M._anvil = Display.image_load("features/sprites/anvil.png")
local UNSOLID_ADD = {
  SourceMap.FLAG_SEETHROUGH,
  M.FLAG_HAS_VAULT
}
local make_legend
make_legend = function(args, legend)
  args.tileset = args.tileset or { }
  local definition = table.merge({
    ['!'] = {
      add = UNSOLID_ADD,
      content = args.tileset.floor,
      on_placement = args.store_placer,
      matches_none = SourceMap.FLAG_SOLID
    },
    ['.'] = {
      remove = { }
    },
    ['+'] = {
      remove = { },
      matches_none = {
        M.FLAG_HAS_VAULT,
        SourceMap.FLAG_SOLID
      }
    },
    ['*'] = {
      add = {
        SourceMap.FLAG_SEETHROUGH,
        M.FLAG_HAS_VAULT
      },
      matches_none = {
        FLAGS_HAS_CONTENT
      },
      content = args.tileset.floor,
      on_placement = args.gold_placer
    },
    ['X'] = {
      add = {
        SourceMap.FLAG_SEETHROUGH,
        M.FLAG_NO_ENEMY_SPAWN
      },
      content = args.tileset.floor,
      on_placement = function(map, xy)
        return MapUtils.spawn_item(map, "Scroll of Experience", 1, xy)
      end
    },
    ['d'] = {
      add = {
        SourceMap.FLAG_SEETHROUGH,
        M.FLAG_HAS_VAULT
      },
      matches_none = {
        FLAGS_HAS_CONTENT
      },
      content = args.tileset.floor,
      on_placement = args.door_placer,
      remove = SourceMap.FLAG_SOLID
    },
    ['@'] = {
      add = {
        SourceMap.FLAG_SEETHROUGH,
        M.FLAG_NO_ENEMY_SPAWN,
        M.FLAG_NO_ITEM_SPAWN,
        M.FLAG_HAS_VAULT
      },
      matches_none = {
        M.FLAG_HAS_VAULT
      },
      content = args.tileset.floor,
      on_placement = function(map, xy)
        local x, y
        x, y = xy[1], xy[2]
        return append(map.player_candidate_squares, {
          x * 32 + 16,
          y * 32 + 16
        })
      end
    },
    ['p'] = {
      add = {
        SourceMap.FLAG_SEETHROUGH,
        M.FLAG_NO_ENEMY_SPAWN,
        M.FLAG_NO_ITEM_SPAWN,
        M.FLAG_HAS_VAULT
      },
      matches_none = {
        M.FLAG_HAS_VAULT
      },
      content = args.tileset.floor,
      on_placement = function(map, xy)
        local x, y
        x, y = xy[1], xy[2]
        if args.player_spawn_area then
          return append(map.player_candidate_squares, {
            x * 32 + 16,
            y * 32 + 16
          })
        end
      end
    }
  }, legend)
  if args.group then
    for k, v in pairs(definition) do
      local has_add = args.add and #args.add > 0
      local has_remove = args.remove and #args.remove > 0
      if has_add or has_remove then
        v.group = args.group
      end
    end
  end
  return definition
end
M.ridge_dungeon = function(args)
  return {
    legend = make_legend(args, {
      ['+'] = {
        remove = { },
        matches_none = {
          M.FLAG_HAS_VAULT,
          SourceMap.FLAG_SOLID
        },
        matches_content = args.door_match_content
      },
      ['f'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_NO_ENEMY_SPAWN,
          M.FLAG_NO_ITEM_SPAWN,
          M.FLAG_HAS_VAULT
        },
        content = args.tileset.floor,
        on_placement = function(map, xy)
          local x, y
          x, y = xy[1], xy[2]
          if args.player_spawn_area then
            return append(map.player_candidate_squares, {
              x * 32 + 16,
              y * 32 + 16
            })
          end
        end
      },
      ['F'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_NO_ENEMY_SPAWN,
          M.FLAG_NO_ITEM_SPAWN,
          M.FLAG_HAS_VAULT
        },
        content = args.tileset.floor
      },
      ['D'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT,
          SourceMap.FLAG_HAS_OBJECT
        },
        matches_none = {
          FLAGS_HAS_CONTENT
        },
        content = args.tileset.floor,
        on_placement = args.dungeon_placer
      },
      ['1'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = args.tileset.wall_alt,
        matches_all = SourceMap.FLAG_SOLID,
        matches_none = {
          FLAGS_HAS_CONTENT
        }
      },
      ['2'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = args.tileset.wall,
        matches_all = SourceMap.FLAG_SOLID,
        matches_none = {
          FLAGS_HAS_CONTENT
        }
      },
      ['w'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = args.tileset.wall_alt,
        matches_none = {
          FLAGS_HAS_CONTENT
        }
      }
    }),
    data = random_choice({
      [=[....+++.......
...wdddw......
..wwpppwww....
.wwppppppwwww.
.wppDpDpDpppw.
.wpppDppppppw.
.wppppppppppw.
.www11111wwww.
..............
]=],
      [=[..........++++..
.......ppppppp..
...111pppppppp..
.11ppppDpppp1...
.11ppDpppp111...
.11Dpppp111.....
.1pppp111.......
.1111111........
]=],
      [=[.........++.....
.....wwwwd++....
...wwwpppddw....
.wwwpDpDpDpw....
.wwpppppppww....
.wwpppppwww.....
.+dpppwww.......
.wwwwwww........
]=],
      [=[.........++.....
.....wwwwd++....
...wwwpppddw....
.wwwpppDpppw....
.wwppDppppww....
.wwDppppwww.....
.wppppwww.......
.ww11www........
]=],
      [=[................
.....wwww+......
...wwwppp++.....
.wwwpppDppw.....
.wwpDpppppw.....
.wwppppDwww.....
.+ppppww1.......
.+wwwwww........
]=]
    })
  }
end
M.sealed_dungeon = function(args)
  return table.merge(M.ridge_dungeon(args), {
    data = random_choice({
      [=[.....wwwwwww....
...111DffffdFFF+
.111fffffffdFFF+
.11ffDffff11....
.11***11111.....
.1f*D*111.......
.1111111........]=],
      [[...wwwwwwwwwww...
..wwffffffffww...
..wffff*fffffw...
..wffff*fffffdFF+
..wff*ff**fffdFF+
wwwffff*ffffww...
wffffff*ffffw....
1fDfDfDffffww....
11ffwwwwwwww.....
.111w............
]],
      [[...wwwwww...
..wwfffww...
..wfffffw...
..wffDffdFF+
..wfffffdFF+
wwwfDffww...
wffffffw....
1fDfffww....
11ffwww.....
.111w.......
]],
      [=[......w++w..
......wFFw..
.....wwddww.
...wwwffffww
.wwwffffffww
.wwfDfDfffww
.ww***wwwww.
.wf*D*www...
.wwwwwww....]=],
      [[...wwwww....
..wwfDfdFF+.
.wwffffdFF+.
.wfDfDfwww..
.wwffwww....
..wwww......
]],
      [=[.......wwwwwww
.....wwwfffffw
...wwwffffffww
...wwfDfffffww
+FFdf***D1111.
+FFdf*D*ff1...
...www11111...]=],
      [=[.......wwwwwww...
.....wwwfffffwww.
...wwwfffffffffww
...wwffffffffffww
+FFdf***ffDf1111.
+FFdf***ffff1111.
+FFdf*D*fffff111.
...wwffffDfffffww
...wwwfffffffffww
.....wwwfffffwww.
.......wwwwwww...]=],
      [=[.......wwwwwww
.....wwwfffffw
...wwwffffDfww
...wwfffffffww
+FFdf***f1111.
+FFdf***f1111.
+FFdf*D*ff111.
...wwfffffffww
...wwwffffDfww
.....wwwfffffw
.......wwwwwww]=]
    })
  })
end
M.skull_surrounded_dungeon = function(args)
  return {
    legend = make_legend(args, {
      ['s'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT,
          M.FLAG_NO_ENEMY_SPAWN
        },
        content = args.tileset.floor,
        on_placement = function(map, xy)
          MapUtils.spawn_decoration(map, M._warning_skull, xy, 0, false)
          if args.player_spawn_area then
            local x, y
            x, y = xy[1], xy[2]
            return append(map.player_candidate_squares, {
              x * 32 + 16,
              y * 32 + 16
            })
          end
        end
      },
      ['e'] = {
        add = {
          M.FLAG_HAS_VAULT,
          SourceMap.FLAG_SEETHROUGH
        },
        content = args.tileset.floor,
        on_placement = args.enemy_placer
      },
      ['D'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT,
          SourceMap.FLAG_HAS_OBJECT
        },
        content = args.tileset.floor,
        matches_none = {
          FLAGS_HAS_CONTENT
        },
        on_placement = args.dungeon_placer
      },
      ['1'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT,
          SourceMap.FLAG_HAS_OBJECT
        },
        content = args.tileset.wall,
        matches_none = {
          FLAGS_HAS_CONTENT
        },
        matches_all = SourceMap.FLAG_SOLID
      },
      ['2'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT,
          SourceMap.FLAG_HAS_OBJECT
        },
        content = args.tileset.wall_alt,
        matches_none = {
          FLAGS_HAS_CONTENT
        },
        matches_all = SourceMap.FLAG_SOLID
      },
      ['W'] = {
        add = {
          M.FLAG_HAS_VAULT,
          SourceMap.FLAG_SOLID
        },
        content = args.tileset.wall,
        matches_none = {
          FLAGS_HAS_CONTENT
        }
      },
      ['w'] = {
        add = {
          M.FLAG_HAS_VAULT,
          SourceMap.FLAG_SOLID
        },
        content = args.tileset.wall_alt,
        matches_none = {
          FLAGS_HAS_CONTENT
        }
      }
    }),
    data = [=[.....111.
++..wwsw1
+wwwwses1
+dssdeDe1
+wwwwses1
++..wwsw1
.....111.]=]
  }
end
M.simple_room = function(args)
  return table.merge(M.skull_surrounded_dungeon(args), {
    data = [=[wwwwwwwwwwwwwwwwwwwwwww
wpppppppppppppppppppppw
wpppppppppppppppepppppw
wppppppppppppppppeppppw
wppppppppppppppppeppppw
wpppppppppppppppppppepw
wpppppppppppppppppppepw
wppppppp@ppppppppeppppw
wpppppppppppppppppppepw
wpppppppppppppppepppppw
wpppppppppppppppppppppw
wwwwwwwwwwwwwwwwwwwwwww
]=]
  })
end
M.hell_dungeon = function(args)
  return table.merge(M.skull_surrounded_dungeon(args), {
    data = [=[..+++..
.wdddw.
.wsssw.
.wdddw.
.wsssw.
.wdddw.
.weeew.
wwdddww
2eeeee2
2DeDeD2
22eee22
.22222.]=]
  })
end
M.graghs_lair_entrance = function(args)
  return table.merge(M.skull_surrounded_dungeon(args), {
    data = [=[..+++..
.WdddW.
.WpppW.
.WdddW.
WWpppWW
WpppppW
WseeesW
WDeseDW
WWpDpWW
.W222W.]=]
  })
end
M.hive_dungeon = function(args)
  return table.merge(M.skull_surrounded_dungeon(args), {
    data = [=[...+...
wwdddww
wDsDsDw
wwwwwww]=]
  })
end
M.crypt_dungeon = function(args)
  return table.merge(M.skull_surrounded_dungeon(args), {
    data = [=[..+++..
.wdddw.
.wsssw.
wwdddww
wsssssw
wDsDsDw
wwsssww
.wwwww.]=]
  })
end
M.anvil_encounter = function(args)
  return {
    legend = make_legend(args, {
      ['a'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.snake.floor,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      },
      ['b'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.snake.floor_alt,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      },
      ['c'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.pebble.floor_alt,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      },
      ['B'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.pebble.floor_alt,
        on_placement = args.boss_placer,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      },
      ['e'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.pebble.floor_alt,
        on_placement = args.enemy_placer,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      },
      ['E'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.snake.floor,
        on_placement = args.enemy_placer,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      },
      ['*'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.pebble.floor_alt,
        on_placement = args.gold_placer,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      },
      ['i'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.pebble.floor_alt,
        on_placement = args.item_placer,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      },
      ['1'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.snake.wall,
        matches_all = SourceMap.FLAG_SOLID,
        matches_none = {
          FLAGS_HAS_CONTENT
        }
      },
      ['d'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT
        },
        remove = SourceMap.FLAG_SOLID,
        content = TileSets.snake.floor,
        on_placement = args.door_placer,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      },
      ['w'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.snake.wall_alt,
        matches_none = {
          FLAGS_HAS_CONTENT
        }
      },
      ['<'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.snake.floor,
        on_placement = function(map, xy)
          return MapUtils.spawn_decoration(map, M._anvil, xy)
        end,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      },
      ['s'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_NO_ENEMY_SPAWN
        },
        content = TileSets.snake.floor,
        on_placement = function(map, xy)
          return MapUtils.spawn_decoration(map, M._warning_skull, xy, 0, false)
        end,
        matches_none = {
          M.FLAG_HAS_VAULT
        }
      }
    }),
    data = [=[................................++...........
...............................wddw..........
...............................waaw..........
...............................waaw..........
...............................waaw..........
.............................wwwddwwwww......
..........................wwwwwwaaaawwww.....
.......................wwwwwwwwaaaaaawwww....
................wwwwwwwwwwwwaaaaaaaaaawww....
...........wwwwwwwwwwwwwwaaaaaaaaaaaaaaww....
.........wwwwwwwwwaaaaaaaaaaabbbbbaaaaaww....
.......111111aaaaaaaaaaaaaabbbbbbbbaaaa111...
.....111111aaaaaaabbbaaaEEaaabbbbaaaaaa111...
....11111aaa<aaaabbbbbaaaaaaaaaaaaaaaa1111...
...1111aaaaaaaaaaabbbaaaEEaaaaaaaaaaa11111...
..1111aaaaabbbbaaaaaaaaaaaa<aaaaaaa11111.....
.1111aaaaabbbbbbaaaaaaaaaaaaaaaaa111111......
.111aa<aaaaabbbaaaaaaaaaaaa111111111111......
1111aaaaaaaaaaaaaaaa111111111111111b11111....
11111111aaaaaaaaaaaasdaaaaaaaaadaabbb111111..
1111111111111111111asdaaaaaaaaadaaabbbb11111.
11111111111111111111111111111111aaaabbbbb1111
11ciccccccccccccc11111111111aaaaaaEaaabbbbb11
11ccccceccicc*ccccccc1111111aaEaaaaaaaabbbb11
11ccceccccecccccicccc111111aaaaaaaaaaaaabbb11
11cccccXcccccc*cccccccdssaaaaaBaaaEaaaaaabb11
11cc*ccciccBcccBccecccdssaaaaaaaaaaaaBaaaab11
11icecccccccceccccccc*dssaaaaaaaaaaaaaa111111
1111cccXcecccBcecccc111111111aaaaa11111111111
..1111ccccc*ccceccc1111111111111111111111....
....1111cccccccccc111........................
......11111111111111.........................
........11111111111..........................]=]
  }
end
M.big_encounter1 = function(args)
  return {
    legend = table.merge(M.anvil_encounter(args).legend, {
      ['P'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT,
          M.FLAG_NO_ENEMY_SPAWN
        },
        content = TileSets.pebble.wall
      },
      ['1'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.pebble.wall,
        matches_all = SourceMap.FLAG_SOLID,
        matches_none = {
          FLAGS_HAS_CONTENT
        }
      }
    }),
    data = [=[..............................................................................wwwwwwwwwww..................
.................................................................wwwwwwwwwwwwwwwwwwwwwwwwwwww..............
......................................................wwwwwwwwwwwwwwwwwwwwwwwwwwaaaaaaaawwwwwww............
.................................................wwwwwwwwwwwwwwwwwaaaaaaaaaaaaaaaEaaaaaaaaawwwww...........
...........................................wwwwwwwwwwwwaaaaaaaaaaaaaaaeaaaaaaaaaaaaaaaaaaaaaawww...........
........................................wwwwwwwwwwaaaaaaaaaaaEaaaa*aaaaaaaaaaaaaaa*aaa*aaaaaaaww...........
....................................wwwwwwwwwaaaaaaaeaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaww...........
..................................wwwwwwwaaaaaEaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaww...........
.................................wwwwwaaaaaaaaaaaaaaaaaaaaaaaEaaaaaaaaaaaaaaaEaaaaaaaEaaaaaaa.ww...........
................................wwwwaaaaaaaeaaaeeaaaaaaeaaaaaaaaaaaaaaaaaaaeaaaaaaaaaaaaeaaaawww...........
................................wwwaaaaaaaaaaaaaaaaaaaaaaaaeaaaaaaaaaaaaaaaaeaaaaaaaaaaaaaaawwww...........
................................wwwaaaaaaEaaaaaaaaaaaaaaaaaaaaaaaaaaaaa*aaaaaaaaaaaaaaaaaawwwwww...........
................................wwwaaaaaaaaaaaaaaaaaaeaaiaaaaa*aaawwwwwwwwwwwwwwwwwwwwwwwwwwwwww...........
.................................wwaaaaaaaiaaaeaaaaaaaaawwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww..ww...........
.................................wwaaaaaeaaaaaaaeaaawwwwwwwwwwwwwwwwwwwwwwww...............................
.................................wwwwwwaeaaaaaaaawwwwwwwwwwwwwww...........................................
.................................wwwwwwaaaaaaaaaawwwwwwww..................................................
.................................wwwwwwaaawwwwwwww.........................................................
.................................wwwwwwaaawwwwwwww.........................................................
.................................wwwwwwdddwwwwwwww.........................................................
.................................wwwwww+++wwwwwwww.........................................................]=]
  }
end
M.big_encounter2 = function(args)
  return {
    legend = table.merge(M.anvil_encounter(args).legend, {
      ['P'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT,
          M.FLAG_NO_ENEMY_SPAWN
        },
        content = TileSets.pebble.wall
      },
      ['1'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = TileSets.pebble.wall,
        matches_all = SourceMap.FLAG_SOLID,
        matches_none = {
          FLAGS_HAS_CONTENT
        }
      }
    }),
    data = [=[.............wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww...
......wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww.
....wwwwwwwwwwwbbbbbbbbbbbbbbbbbbbbbbbbbbiwwwwwwbbbbbbbbbbww
.wwwwwwwbbbbbbbbbebbbbbebbbbbbbbbbbbbbbbbbbbbbbbbbbbebbbbbbw
wwwwwwbbbbbbbbbbbbbbbbbbbb*bbbbbbbbbebbbbbbbbbbbbbbbbbebbbbw
wwwbbbbbbbebbbbbbbbbbbbbbbbbbbbbbbbbbibbbbbbbbbbbbbebbbbbbbw
wwbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbeebbw
wwbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbw
wwwwbbbbbbbbbbbbbbwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwbbbbbw
wwwwwwbbbbbbbbbbwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwbbbw
..wwwwwwaaaaaaaawww.......................wwwwwwwwwwwwwwwwbw
....wwwwwaaaaaawww....................................wwwwww
......wwwwaaaaaww.......................................wwww
........wwwaaawww.........................................w.
........wwwaaawww...........................................
.........wwdddww............................................
.........ww+++ww............................................
]=]
  }
end
M.tunnel = function(args)
  return {
    legend = table.merge(M.anvil_encounter(args).legend, {
      ['w'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_NO_ENEMY_SPAWN,
          M.FLAG_HAS_VAULT
        },
        matches_none = {
          FLAGS_HAS_CONTENT
        },
        content = TileSets.snake.wall
      },
      ['G'] = {
        remove = { },
        matches_none = {
          M.FLAG_HAS_VAULT,
          SourceMap.FLAG_SOLID
        },
        matches_content = TileSets.pebble.floor
      }
    }),
    data = random_choice({
      [=[.....wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww.
.wwwwwwccccccccccc*cccccccccccccccc*cccccccccccw.
+dccccceccccccc*cccccccccccccccccecccccccccccccd+
+dcccccccccccccccccccccccccccccccccccccccccccccd+
.wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww.
]=],
      [=[.....wwwwwww.....................................
.wwwwwwccccw.....................................
Gdcccccecccw.....................................
.wwwwwwwcccw.....................................
.....wwwccwwwwwwwwwwwwwwwwwwwwwww................
......wwccccccccccccccccccccccccwwwwwwwwwwwwwwww.
......wwwwwwwwwccccccccccccccccccccccccccccccccd+
............wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww.
]=]
    })
  }
end
M.honeycomb = function(args)
  return {
    legend = table.merge(M.anvil_encounter(args).legend, {
      ['w'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_NO_ENEMY_SPAWN,
          M.FLAG_HAS_VAULT
        },
        matches_none = {
          FLAGS_HAS_CONTENT
        },
        content = TileSets.hive.combs
      },
      ['i'] = {
        add = UNSOLID_ADD,
        on_placement = args.item_placer,
        matches_none = SourceMap.FLAG_SOLID
      }
    }),
    data = random_choice({
      [=[.w..w..
ww..ww.
.......
..i....
ww..ww.
.w..w..
]=],
      [=[.......
...w...
.wwwww.
.wi..w.
...w...
...w...
]=],
      [=[.......
..wwww.
.wi....
.wiwww.
.w.....
.......
]=],
      [=[.....
.w...
.w...
...w.
]=],
      [=[.....
..w..
.www.
..w..
.....
]=],
      [=[w....w
.w..w.
......
.w..w.
w....w
]=]
    })
  }
end
M.stone_henge = function(args)
  return {
    legend = table.merge(M.anvil_encounter(args).legend, {
      ['w'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_NO_ENEMY_SPAWN,
          M.FLAG_HAS_VAULT
        },
        matches_none = {
          FLAGS_HAS_CONTENT
        },
        content = TileSets.snake.wall
      },
      ['p'] = {
        add = UNSOLID_ADD,
        content = TileSets.snake.floor,
        matches_none = SourceMap.FLAG_SOLID
      },
      ['P'] = {
        add = UNSOLID_ADD,
        content = TileSets.grass.floor_alt2,
        matches_none = SourceMap.FLAG_SOLID
      },
      ['s'] = {
        add = UNSOLID_ADD,
        content = TileSets.snake.floor,
        matches_none = SourceMap.FLAG_SOLID,
        on_placement = args.store_placer
      },
      ['S'] = {
        add = UNSOLID_ADD,
        content = TileSets.grass.floor_alt2,
        matches_none = SourceMap.FLAG_SOLID,
        on_placement = args.store_placer
      },
      ['i'] = {
        add = UNSOLID_ADD,
        content = TileSets.grass.floor_alt2,
        on_placement = args.item_placer,
        matches_none = SourceMap.FLAG_SOLID
      }
    }),
    data = random_choice({
      [=[.w..w..
wwppww.
.pssp+.
.pssp+.
wwppww.
.w..w..
]=],
      [=[..PPPPPPPPPPPP...
.PPPPPPPPPPPPPP..
.PPPSPPPPPPSPPPP.
.PPPPPPPPPPPPPPP.
.PPPPPPPPPPPPPPP.
PPPPPPPPPPPPPPPP.
PPPPSPPPPPPSPPPP.
PPPPPPPPPPPPPPPP.
PPPPPPPPPPPPPPP..
.PPPPPPPPPPPPP...
]=],
      [=[+PPPPPPPPP+
PPwwPwPwwPP
PPwSPwPSwPP
PPPPPPPPPPP
PPwwPiPwwPP
PPPPPPPPPPP
PPwSPwPSwPP
PPwwPwPwwPP
+PPPPPPPPP+
]=],
      [=[ww.w.ww.
wSPwPSw.
.PPPPP+.
wwPiPww.
.PPPPP+.
wSPwPSw.
ww.w.ww.
]=],
      [=[.w...w..
wwPPPww.
.PSPSP+.
.PPiPP+.
.PSPSP+.
wwPPPww.
.w...w..
]=]
    })
  }
end
M.cavern = function(args)
  return {
    legend = table.merge(M.anvil_encounter(args).legend, {
      ['w'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_NO_ENEMY_SPAWN,
          M.FLAG_HAS_VAULT
        },
        matches_none = {
          FLAGS_HAS_CONTENT
        },
        content = TileSets.snake.wall
      }
    }),
    data = random_choice({
      [=[..........wwwwwwwww.......
.....wwwwwwaaaaaawwwwww...
..wwwwaaaaaaaaaaaaaaaawww.
wwwaaaaaaaa!aaaaaaaaaaaaww
waaaaaaaaaaaaaaaaaaaaaaww.
wwwaaaaaaaaaaaaaaaaaaaww..
wwwwwwwwwww....++.........
]=],
      [=[.....wwwwwww.......
.....waaaawwwwww...
..wwwwaaaaaaaaaww..
wwwaaaaa!aaaaaaaw..
wwwaaaaaaaaaaaaww..
wwwwww..++.........
]=],
      [=[.....wwwwwwwww.......
.....waaaaaawwwwww...
..wwwwaaaaaaaaaaawww.
wwwaaa!aaaaaaaaaaaaww
wwwaaaaaaaaaaaaaaww..
wwwwww....++.........
]=]
    })
  }
end
M.small_random_vault = function(args)
  return {
    legend = make_legend(table.merge(args, {
      store_placer = random_choice({
        do_nothing,
        args.store_placer
      })
    }), {
      ['*'] = {
        add = UNSOLID_ADD,
        content = args.tileset.floor,
        on_placement = random_choice({
          do_nothing,
          args.gold_placer
        }, {
          matches_none = FLAGS_HAS_CONTENT
        })
      },
      ['i'] = {
        add = UNSOLID_ADD,
        content = args.tileset.floor,
        on_placement = random_choice({
          do_nothing,
          args.item_placer
        }, {
          matches_none = FLAGS_HAS_CONTENT
        })
      },
      ['e'] = {
        add = UNSOLID_ADD,
        content = args.tileset.floor,
        on_placement = random_choice({
          do_nothing,
          args.enemy_placer
        }, {
          matches_none = FLAGS_HAS_CONTENT
        })
      },
      ['p'] = {
        add = UNSOLID_ADD,
        content = args.tileset.floor,
        matches_none = M.FLAG_HAS_VAULT
      },
      ['d'] = {
        add = {
          SourceMap.FLAG_SEETHROUGH,
          M.FLAG_HAS_VAULT,
          SourceMap.FLAG_HAS_OBJECT
        },
        remove = SourceMap.FLAG_SOLID,
        content = args.tileset.floor_alt,
        on_placement = function(map, xy)
          return MapUtils.spawn_door(map, xy)
        end
      },
      ['w'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = args.tileset.wall_alt,
        matches_none = FLAGS_HAS_CONTENT
      }
    }),
    data = random_choice({
      [=[...++++++
..++wwww+
++www!ew+
+wwe**ew+
+wppppww+
+wddwww++
++++++++.]=],
      [=[..+++++++.
.+wwwwww+.
++wepp!w+.
+wwpiipw+.
+wppeppw+.
+wddwwww+.
+++++++++.]=],
      [=[..+++++++
.+wwwwww+
++wppeew+
+wwppppw+
+wppp!pw+
+wddwwww+
+++++++++]=],
      [=[..++++++.
.+wwwww++
++wpppwe+
+wwpwpp*+
+wppwp!e+
+wddwwww+
+++++++++
]=],
      [=[.+++++++.
.+wwwww++
++wpppww+
+wwpwe*w+
+wppwe!w+
+wddwwww+
+++++++++]=],
      [=[.+++++++.
.+wwwww++
++wpeeww+
+wwpwe!w+
+wppwppw+
+wddwddw+
+++++++++
]=],
      [=[.+++++++.
.+wwwww++
++wpeeww+
+dwpwe!d+
+di*wppd+
+wwwwwww+
+++++++++
]=]
    })
  }
end
M.small_item_vault = function(args)
  return {
    legend = make_legend(args, {
      ['*'] = {
        add = UNSOLID_ADD,
        content = args.tileset.floor,
        on_placement = args.gold_placer,
        matches_none = FLAGS_HAS_CONTENT
      },
      ['i'] = {
        add = UNSOLID_ADD,
        content = args.tileset.floor,
        on_placement = args.item_placer,
        matches_none = FLAGS_HAS_CONTENT
      },
      ['e'] = {
        add = UNSOLID_ADD,
        content = args.tileset.floor,
        on_placement = args.enemy_placer,
        matches_none = FLAGS_HAS_CONTENT
      },
      ['p'] = {
        add = UNSOLID_ADD,
        content = args.tileset.floor,
        matches_none = FLAGS_HAS_CONTENT
      },
      ['d'] = {
        add = UNSOLID_ADD,
        remove = SourceMap.FLAG_SOLID,
        content = args.tileset.floor_alt,
        on_placement = args.door_placer
      },
      ['w'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = args.tileset.wall_alt,
        matches_none = FLAGS_HAS_CONTENT
      },
      ['1'] = {
        add = {
          SourceMap.FLAG_SOLID,
          M.FLAG_HAS_VAULT
        },
        content = args.tileset.wall_alt,
        matches_all = SourceMap.FLAG_SOLID,
        matches_none = {
          FLAGS_HAS_CONTENT
        }
      }
    }),
    data = random_choice({
      [=[++++++
+wwww+
+wipd+
+wwww+
++++++]=],
      [=[+++++
+www+
+wid+
+www+
+++++]=]
    })
  }
end
M.crypt_encounter_vault = function(args)
  return table.merge(M.small_item_vault(args), {
    data = random_choice({
      [=[.www.......
wwpwwwwwww.
1ieeepdppd+
1ieeepdppd+
wwppppwwww.
.wwwwww....]=],
      [=[wwwwwww.
1ppppww.
1ieepd+.
1ieepd+.
1ppppww.
wwwwwww.]=],
      [=[wwwwwww.
1ppppp1.
1ieeepd+
1ieeepd+
1ppppp1.
wwwwwww.]=]
    })
  })
end
M.small_item_vault_multiway = function(args)
  return table.merge(M.small_item_vault(args), {
    data = random_choice({
      [=[+++++
+wdw+
+did+
+wdw+
+++++]=]
    })
  })
end
M.hell_entrance_vault = function(args)
  return table.merge(M.small_item_vault(args), {
    data = random_choice({
      [=[+++++++...
+wpwww++..
+ppppww+++
+ppp*pwww+
+pppipppd+
+pppppppd+
+wdwwwwww+
++++++++++]=],
      [=[+++++++++
+wpwwwww+
+dpp*ppp+
+dpppppp+
+dppippp+
+wdwwwww+
+++++++++]=],
      [=[++++++++++
+wdwwwwww+
+pppppppd+
+p*pppppd+
+pppipppd+
+pppppppd+
+wpwwwwww+
++++++++++]=]
    })
  })
end
M.crypt_entrance_vault = function(args)
  return table.merge(M.small_item_vault(args), {
    data = random_choice({
      [=[+++++++...
+wdwww++..
+dpppww+++
+dpp*pwww+
+dppipppd+
+dppppppd+
+wdwwwwww+
++++++++++]=],
      [=[+++++++++
++wwwww++
+wwp*pww+
+dpppppd+
+dpppppd+
+wwpipww+
++wwdww++
+++++++++]=],
      [=[+++++++++
++wwwww++
+wwp*pww+
+dpppppd+
+wwpipww+
++wwdww++
+++++++++]=],
      [=[+++++++++
+wdwwwww+
+dpp*ppd+
+dpppppd+
+dppippd+
+wdwwwww+
+++++++++]=],
      [=[++++++++++
+wdwwwwww+
+dppppppd+
+d*pppppd+
+dppipppd+
+dppppppd+
+wdwwwwww+
++++++++++]=]
    })
  })
end
M.hive_entrance_vault = function(args)
  return table.merge(M.crypt_entrance_vault(args), {
    data = random_choice({
      [=[+++++++++
++wwwww++
+wwp*pww+
+dpppppd+
+wwpipww+
++wwdww++
+++++++++]=]
    })
  })
end
M.dungeon_tunnel = function(args)
  return table.merge(M.small_item_vault(args), {
    data = random_choice({
      [=[...wwwwwwwwwwwww......
.wwwpppppppppppwwwwww.
+dpppepepepepppdppppw.
.wwppppppppppppwwwwpd+
..wwpppppppppppw..www.
...wwwwwwwwwwwww......
]=],
      [=[..........+.....
...wwwwwwwdwwwww
.wwwpppppppppppw
.wpppepepepepppw
.wwppppppppppppw
..wwpppppppppppw
...wwwwwwwdwwwww
..........+.....
]=],
      [=[...wwwwwwwwwwwww......
.wwwpppppppppppwwwwww.
.wpppepepepepppdppppw.
.wwppppppppppppwwwwpd+
..wwpppppppppppw..www.
...wwwwwwwdwwwww......
..........+...........
]=]
    })
  })
end
return M
