import ".General" -- For 'append'

local Templates = import "@codegen.Templates"

local M = nilprotect {} -- Submodule

local function typedef_aux(...) local namespaces = {...} ; return function (...)
    if type(...) == "table" then
        table.insert_all(namespaces, {...}) ;  return typedef_aux(unpack(namespaces))
    end ; local name = ...; assert(#{...} == 1)
    return function(definition)
        local T = Templates.type_parse(definition, namespaces, name)
        local type = Templates.compile_type(T)
        if name then namespaces[#namespaces][name] = type end
        return type
    end
end end

function typedef(...)
    if #{...} == 1 then
        return typedef_aux()(false)(...) -- Simple type 
    else return typedef_aux(...) end
end