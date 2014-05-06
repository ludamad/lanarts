local Apts = import "@stats.AptitudeTypes"
local Traits = import ".ItemTraits"
local ring_define = (import ".ItemDefineUtils").ring_define
local StatContext = import "@StatContext"
local Proficiency = import "@Proficiency"
local ItemRandomDescriptions = import ".ItemRandomDescriptions"

local random_ring_names = data_load("random_ring_names", {})

local function aptitude_bonus_ring_define(args)
    args.needs_identification = true

    function args.on_calculate(self, stats)
        StatContext.add_all_aptitudes(stats, args.aptitude, self.real_bonus or self.bonus) -- real_bonus can be a full 4-vector, while bonus must be a number
    end

    function args.on_map_init(self)
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

        self.unidentified_name = random_ring_names[args] or ItemRandomDescriptions.random_ring_name()
        random_ring_names[args] = self.unidentified_name

        self.unidentified_description = "You're not sure what it does, if anything."
    end

    return ring_define(args)
end

local APTS = {
    {Apts.RANGED,   "Archer's Ring",    {4,8}, "A ring that provides the user with cunning during Ranged combat."},
    {Apts.SLASHING, "Ring of Slashing", {3,6}, "A ring that provides the user with skill in Slashing weaponry and combat."},
    {Apts.PIERCING, "Ring of Piercing", {3,6}, "A ring that provides the user with skill in Piercing weaponry and combat."},
    {Apts.BLUNT,    "Ring of Blunting", {3,6}, "A ring that provides the user with skill in Blunt weaponry and combat."}
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

local APTS = {
    {Apts.RANGED,   "Archer's Ring",    {4,8}, "A ring that provides the user with cunning during Ranged combat."}
}