for vpath in values(find_submodules("unstable.stats", true, "Define*")) do
    import(vpath)
end

-- Iterate over all unstable/../DefineXX.lua files and add them as separate tests
for file in values(find_submodules("unstable", true, "Define*")) do
    TestCases["(import \"" .. file .. "\")"] = function()
        import(file)
    end
end
