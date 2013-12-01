local StatML = import "statml.StatML"

local M = nilprotect {} -- Submodule

local MODULE = "dungenerate"
local DATA_MODULE = "dungenerate-data"

local TEST_ROOT = MODULE .. '.tests.' 
local function not_test_submodule(vpath)
    return vpath:sub(1,#TEST_ROOT) ~= TEST_ROOT
end

local function resolve_dependent_parsers()
    local files = find_submodules(MODULE, true, nil, not_test_submodule)
    for c in values(files) do
        local module = import(c)
        -- Signal to modules that we have finished loading
        if module and rawget(module, "statml_onload") then
            module.statml_onload()
        end
    end
end

function M.main(args)
    import_all(MODULE, true, nil, not_test_submodule)
    StatML.load_directory("game/"..DATA_MODULE)
--    resolve_dependent_parsers()
    StatML.parse_all()
end

return M