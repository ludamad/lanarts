local content_files = {
    "unstable.items.DefineConsumables",
    "unstable.items.DefineWeapons",
    "unstable.content.aptitude_types",
    "unstable.content.cooldown_types",
    "unstable.content.item_traits",
    "unstable.content.races",
    "unstable.content.skills",
    "unstable.content.spells",
    "unstable.content.stat_utils",
    "unstable.content.monsters.animals",
    "unstable.content.status_types",

    "unstable.items.equipment_types",
    "unstable.items.items_consumables"
}

for file in values(content_files) do
    TestCases["(import \"" .. file .. "\")"] = function()
        import(file)
    end
end
