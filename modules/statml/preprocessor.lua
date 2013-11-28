local StatML -- Lazily imported
local function lazy_import() 
    StatML = StatML or import "@StatML"
end

local M = nilprotect {} -- Submodule

local preproc_fenv = setmetatable({
    ids = function(tag)
        return (" "):join(StatML.id_list(tag))
    end
},{
    __index = function(_, id)
        -- Dynamic resolve amongst all instances
        local result = StatML.resolve_node(id)
        if not result then
            error("Problem in macro, '" .. id .. "' does not specify a valid instance.")
        end
        return result
    end
})

local function macro_call(macro, --[[Optional]] allow_nonstring)
    local func_loader, err = loadstring("return " .. macro)
    if err then errorf("Error in macro '%s':\n%s", macro, err) end
    setfenv(func_loader, preproc_fenv)
    local result = func_loader()
    if not allow_nonstring and type(result) ~= "string" then
        errorf("Error in macro '%s':\n\texpecting 'string' but got type '%s': '%s'", 
            macro, type(result), tostring(result)
        )
    end
    return result
end
        
local PREPROC_MATCHER = "($%b{})"
local PREPROC_LINE_MATCHER = "^%s*".. PREPROC_MATCHER .. "%s*$"

-- Strip surrounding '${...}'
local function macro_strip(str) return str:sub(3, -2) end

function M.preprocess(data)
    lazy_import()
    if type(data) ~= "string" 
        -- TODO: Go into tables and preprocess fields
        then return data
    end
    local linematch = data:match(PREPROC_LINE_MATCHER)
    if linematch then 
        return macro_call(macro_strip(linematch), --[[Allow non-string]] true) 
    end
    return data:gsub(PREPROC_MATCHER, function(str)
        return macro_call(macro_strip(str))
    end)
end

return M