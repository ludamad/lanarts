local M = nilprotect {} -- Submodule

local StatMLClass = typedef [[
    name :string
]]

local StatMLContext = typedef [[
    classes :list
]]

local function parse_class_candidate(context, line, fields)
    -- Returns false if was not a class description.
    local class_name = line:match("^class%s+([%w_]+)")
    if not class_name then return false end
    local class = StatMLClass.create(class_name)
    
end

return M