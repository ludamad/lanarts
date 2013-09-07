local content_files = {
    "unstable.content.aptitude_types",
    "unstable.content.cooldown_types",
    "unstable.content.item_traits",
    "unstable.content.items",
    "unstable.content.races",
    "unstable.content.skills",
    "unstable.content.spells",
    "unstable.content.stat_utils",
    "unstable.content.monsters.animals",
    "unstable.content.status_types"
}

for file in values(content_files) do
    TestCases["(import \"" .. file .. "\")"] = function()
        import(file)
    end
end
