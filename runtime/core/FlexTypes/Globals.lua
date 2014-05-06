local Flex = import "@FlexTypes"
local Struct = import "@Struct"

local function assert_all_tables(...) 
    for t in values{...} do assert(type(t) == "table", "Misplaced type name in typedef!") end
end

local function typedef_aux(...) local namespaces = {...} ; assert_all_tables(...) ; return function (...)
    if type(...) == "table" then
        assert_all_tables(...)
        table.insert_all(namespaces, {...}) ; return typedef_aux(unpack(namespaces))
    end ; local name = ...; assert(#{...} == 1, "Incorrect name field.")
    return function(definition)
        local S = Struct.create(name)
        local C = Flex.context_create(namespaces, S)
        local T = Templates.type_parse(definition, namespaces, name)
        local type = Templates.compile_type(T)
        if name then namespaces[#namespaces][name] = type end
        return type
    end
end end

function typedef2(...)
    if #{...} == 1 and type(...) == "string" then
        return typedef_aux()(false)(...) -- Simple type 
    else return typedef_aux(...) end
end