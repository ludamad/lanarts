import ".General" -- For 'append'

local Builders = import "@codegen.Builders"

local M = nilprotect {} -- Submodule

function typedef(definition_string)
    local builder = Builders.type_parse(definition_string)
    return builder:compile()
end