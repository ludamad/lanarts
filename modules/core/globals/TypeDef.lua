import ".General" -- For 'append'

local TypeCompiler = import "@codegen.TypeCompiler"

local M = nilprotect {} -- Submodule

local C = TypeCompiler.create()
function typedef(definition_string)
    return C:compile(definition_string)
end