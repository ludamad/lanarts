--------------------------------------------------------------------------------
-- StatML interaction
--------------------------------------------------------------------------------

local StatML = import "statml.StatML"

StatML.object_parsedef("Pos", [[
    x,y :float
]])

StatML.class_parsedef("Actor", [[
    Pos as xy
]])

StatML.load_and_parse_file(path_resolve "demoshoot.yaml")

print("Example " .. tostring(StatML.get "Example"))

--------------------------------------------------------------------------------
-- StatML interaction.
--------------------------------------------------------------------------------
