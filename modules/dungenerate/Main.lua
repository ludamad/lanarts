local StatML = import "statml.StatML"

local M = nilprotect {} -- Submodule

local MODULE = "dungenerate"
local DATA_MODULE = "dungenerate-data"

local function resolve_parsers()
    local files = find_submodules(MODULE, true, "*.lua")
    for c in values(files) do
        if not filter or filter(c) then
            local module = import(c)
            -- Signal to modules that we have finished loading
            if module and rawget(module, "statml_onload") then
                module.statml_onload()
            end
        end
    end
end

function M.main(args)
    import_all(MODULE)
    StatML.parse_all("modules/"..DATA_MODULE)
    print "Test"
    StatML.assert_finished()
end

return M