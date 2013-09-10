local Apts = import "@stats.AptitudeTypes"
local Traits = import ".ItemTraits"
local ring_define = (import ".ItemUtils").ring_define
local StatContext = import "@StatContext"
local Proficiency = import "@Proficiency"
local ItemRandomDescriptions = import ".ItemRandomDescriptions"

ring_define {
    lookup_key = "Ring of Slashing",
    sprite = {}, -- TODO
    description = function(self)
        return ("A ring that confers expertise in Slashing weaponry. Provides %s aptitude."):format(self.bonus)
    end,
    on_calculate = function(self, stats)
        StatContext.add_all_aptitudes(stats, Apts.SLASHING, self.bonus)
    end,
    needs_identification = true, -- See below
    on_init = function(self)
        local P = Proficiency
        assert(self.bonus)
        self.identify_requirements = {P.proficiency_requirement_create({Apts.MAGIC_ITEMS}, self.bonus + 1)}
        self.name = '+' .. self.bonus .. ' ' .. self.lookup_key

        self.unidentified_name = ItemRandomDescriptions.random_ring_name()
        self.unidentified_description = "You're not sure what it does, if anything."
    end
}
