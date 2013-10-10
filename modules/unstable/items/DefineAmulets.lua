local Apts = import "@stats.AptitudeTypes"
local Traits = import ".ItemTraits"
local ring_define = (import ".ItemDefineUtils").ring_define
local StatContext = import "@StatContext"
local Proficiency = import "@Proficiency"
local ItemRandomDescriptions = import ".ItemRandomDescriptions"

local random_amulet_names = data_load("random_amulet_names", {})

local function aptitude_bonus_ring_define(args)
    args.needs_identification = true

    function args:on_action(user, action, target, source)
        
    end

    function args:on_init()
        local P = Proficiency
        assert(self.bonus)
        for k in values{"identify_requirements", "proficiency_requirements"} do
            self[k] = {
                P.proficiency_requirement_create(
                    {Apts.MAGIC_ITEMS}, 
                    self.bonus + random_resolve(args.difficulty)
                ),
                P.proficiency_requirement_create(
                    {args.aptitude}, 
                    self.bonus
                )
            }
        end
        self.name = '+' .. self.bonus .. ' ' .. self.lookup_key

        self.unidentified_name = random_amulet_names[args] or ItemRandomDescriptions.random_ring_name()
        random_amulet_names[args] = self.unidentified_name

        self.unidentified_description = "You're not sure what it does, if anything."
    end

    return ring_define(args)
end

local APTS = {
    {Apts.FIRE,   "Talisman of Incineration",    {4,8}, "An amulet that causes fire damage on every melee attack."},
}

for apt in values(APTS) do
    local APTITUDE, NAME, DIFFICULTY, DESCRIPTION = unpack(apt)
    aptitude_bonus_ring_define {
        lookup_key = NAME,
        aptitude = APTITUDE,
        sprite = {}, -- TODO
        difficulty = DIFFICULTY,
    
        description = function(self)
            return ("%s. Provides %s aptitude."):format(DESCRIPTION, self.bonus)
        end
    }
end