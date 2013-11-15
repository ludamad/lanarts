local Types = import "core.Types"

local Map = Types.new [[
    objects :list
    label :string
    id :int
    x, y, depth :float
    solid, destroyed :bool

    size :getter {x,y}
]]

GameObjectBase.on_step, GameObjectBase.on_draw = do_nothing,do_nothing 

return Map

--
--private:
--    std::string _label;
--    level_id _levelid;
--    int _steps_left;
--    Size _size;
--    GameTiles _tiles;
--    GameInstSet _inst_set;
--    MonsterController _monster_controller;
--    CollisionAvoidance _collision_avoidance;
--    /* Used to store dynamic drawable information */
--    LuaDrawableQueue _drawable_queue;
--
--    bool _is_simulation;
--};
