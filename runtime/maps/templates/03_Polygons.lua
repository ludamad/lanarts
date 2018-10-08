return {
  version = "1.2",
  luaversion = "5.1",
  tiledversion = "1.2.0",
  orientation = "orthogonal",
  renderorder = "right-down",
  width = 100,
  height = 100,
  tilewidth = 32,
  tileheight = 32,
  nextlayerid = 3,
  nextobjectid = 7,
  properties = {},
  tilesets = {},
  layers = {
    {
      type = "tilelayer",
      id = 1,
      name = "Tile Layer 1",
      x = 0,
      y = 0,
      width = 100,
      height = 100,
      visible = true,
      opacity = 1,
      offsetx = 0,
      offsety = 0,
      properties = {},
      encoding = "lua",
      chunks = {}
    },
    {
      type = "objectgroup",
      id = 2,
      name = "Object Layer 1",
      visible = true,
      opacity = 1,
      offsetx = -897,
      offsety = -410,
      draworder = "topdown",
      properties = {},
      objects = {
        {
          id = 2,
          name = "front",
          type = "",
          shape = "polygon",
          x = 102,
          y = 6,
          width = 0,
          height = 0,
          rotation = 0,
          visible = true,
          polygon = {
            { x = -481, y = -64 },
            { x = -275, y = 94 },
            { x = -261, y = 205 },
            { x = -462, y = 250 },
            { x = -582, y = 170 },
            { x = -653, y = 18 },
            { x = -598, y = -82 }
          },
          properties = {}
        },
        {
          id = 3,
          name = "body",
          type = "",
          shape = "polygon",
          x = 103,
          y = 7,
          width = 0,
          height = 0,
          rotation = 0,
          visible = true,
          polygon = {
            { x = -481, y = -64 },
            { x = -457, y = -162 },
            { x = -385, y = -243 },
            { x = -286, y = -282 },
            { x = -153, y = -302 },
            { x = 0, y = -294 },
            { x = 144, y = -243 },
            { x = 234, y = -159 },
            { x = -275, y = 95 }
          },
          properties = {}
        },
        {
          id = 6,
          name = "back",
          type = "",
          shape = "polygon",
          x = 65,
          y = -12,
          width = 0,
          height = 0,
          rotation = 0,
          visible = true,
          polygon = {
            { x = -20, y = -10 },
            { x = 252, y = -146 },
            { x = 478, y = 98 },
            { x = 372, y = 208 },
            { x = 234, y = 256 },
            { x = 76, y = 238 },
            { x = 10, y = 140 }
          },
          properties = {}
        }
      }
    }
  }
}
