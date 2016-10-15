--[[------------------------------------------------------

  box2d bindings generator
  ------------------------

  This uses the 'dub' tool and Doxygen to generate the
  bindings for Box2D.

  Input:  headers in 'vendor'
  Output: cpp files in 'src'

--]]------------------------------------------------------
require 'lubyk'
require 'dub'

local base = lk.pathDir(lk.scriptDir())
local b2_base = base .. '/vendor/Box2D'

local ins = dub.Inspector {
  INPUT   = {
    b2_base .. '/Box2D/Common',
    b2_base .. '/Box2D/Collision',
    b2_base .. '/Box2D/Collision/Shapes',
    b2_base .. '/Box2D/Dynamics',
    -- A bug in dub prevents this from working
    --b2_base .. '/Box2D/Dynamics/Contacts',
    b2_base .. '/Box2D/Dynamics/Joints',
    base    .. '/include',
  },
  ignore = {
    -- FIXME: Why cannot dub ignore classes here ?
    b2Draw = {
      'DrawPolygon',
      'DrawSolidPolygon',
      'DrawCircle',
      'DrawSolidCircle',
      'DrawSegment',
      'DrawTransform',
    },
  },
}

local binder = dub.LuaBinder()

-- Renamge b2Xyz classes to b2.Xyz
function binder:name(elem)
  local name = elem.name
  if name then
    name = string.match(name, '^b2(.+)') or name
  end
  return name
end

function binder:constName(name)
  name = string.match(name, '^b2_(.+)') or name
  return name
end

binder:bind(ins, {
  output_directory = base .. '/src/bind',
  custom_bindings  = base .. '/bind',
  ignore = {
    'b2TreeNode',
    -- Incomplete implementation
    'b2WheelJoint',
    'b2WheelJointDef',
  },
  -- Remove this part in headers
  -- LuaBinder 115
  header_base = {b2_base, base .. '/include'},
  -- Execute all lua_open in a single go
  -- with luaopen_b2_vendor (creates b2_vendor.cpp).
  single_lib = 'b2',
  -- Other name so that we can first load b2.lua
  luaopen = 'b2_vendor',
})


