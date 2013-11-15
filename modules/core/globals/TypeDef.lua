import ".General" -- For 'append'

local M = nilprotect {} -- Submodule

local Field = newtype()
function Field:init(name, type)
    self.name, self.type = name, type
end

local TypeBuilder = newtype()

function TypeBuilder:parse_line(line)
    local parts = line:split(":")
    assert(#parts == 2)
    local var_parts = parts[1]:split(",")
    local type = parts[2]:trim()
    for v in values(var_parts) do
        append(self.fields, Field.create(v:trim(), type))
    end
end
function TypeBuilder:init()
	self.fields = {}
end

function TypeDef(definition_string)
    local builder = TypeBuilder.create()
    local lines = definition_string:split("\n")
    for line in values(lines) do
        builder:parse_line(line)
    end
end