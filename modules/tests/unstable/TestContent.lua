local content_files = {
    "unstable.items.DefineConsumables",
    "unstable.items.DefineWeapons"
--[[    "unstable.content.AptitudeTypes",
    "unstable.content.CooldownTypes",
    "unstable.content.ItemTraits",
    "unstable.content.Races",
    "unstable.content.Skills",
    "unstable.content.Spells",
    "unstable.content.StatUtils",
    "unstable.content.Monsters.Animals",
    "unstable.content.StatusTypes",

    "unstable.items.EquipmentTypes",
    "unstable.items.ItemsConsumables"
]]
}

for file in values(content_files) do
    TestCases["(import \"" .. file .. "\")"] = function()
        import(file)
    end
end
