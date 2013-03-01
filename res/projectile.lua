-- What do we need to provide ?
-- 1. Uniform interface for customizing GameInst objects?
    -- be it enemy's, dungeon features, etc
-- 2. Special callbacks depending on the GameInst type?
    -- can greatly simplify interaction with the engine
    -- What if these callbacks -aren't- special ?
    -- categorizing is important, but not much past that
    
-- ok so requirements
-- 1. Easily query the category of an object
-- 2. Easily extend the default capabilites of objects
-- 3. CPP objects & lua objects should blend seemlessly

-- 'room.instance_radius_test'
-- we need to be a hybrid gameinst & lua object

local instances = {
    PROJECTILE = 0,
    ENEMY = 1,
    DUNGEON_FEATURE = 2
}

local visual_only = true

local ProjectileInstance = instances.newtype({ category = instances.PROJECTILE })

local function init_base(self, ...)
    return self.parent.init(self, ...)
end

-- Cannot be directly instantiated ?
-- Although I don't see why not... maybe room.create(ProjectileInstance, ...) is cleaner
-- eg room.create(instances.PlayerInstance, ...)

-- Well. We need to encapsulate a given room.

function ProjectileInstance:init()
    instances.init(self)
end

function ProjectileInstance:step()
    
end

-- step(level) ??
-- Now we are getting into GameState territory
-- It makes sense, but it would have to be passed to -everything-
-- Level table is bad name anyway, we call them floors
-- maybe not toooo bad. 'floor.'

-- Ideally, the running of a lua script should occur on a single level.
-- The same way that a lua ascript occurs with a single GameState context.
-- This is strongly in-line with the convenience of lua scripting.

-- Of the given names, 'room' is the least clashing.
-- But should the global instance be called 'room' ? This takes away a short name from being used conveniently
-- Then again -- we wont often have many room locals, and they still *can* be done
-- OK, so room. 

world.run_on_level(world.rooms[2])

room.instance(1)
