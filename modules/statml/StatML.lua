local yaml = import "core.yaml"

local M = nilprotect {} -- Submodule

local StatMLContext = typedef [[
    classes :map
]]
M.StatMLContext = StatMLContext

function StatMLContext:parse(file)
    local tree = yaml.load(file_as_string(path_resolve "aptitudes.yaml"))
        
end


--local function parse_class_candidate(context, line, fields)
--    -- Returns false if was not a class description.
--    local class_name = line:match("^class%s+([%w_]+)")
--    if not class_name then return false end
--    local class = StatMLClass.create(class_name)
--end

return M