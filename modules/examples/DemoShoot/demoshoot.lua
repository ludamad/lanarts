local M = nilprotect {} -- Submodule

--------------------------------------------------------------------------------
-- StatML schemas
--------------------------------------------------------------------------------

local StatML = import "statml.StatML"

local Pos = StatML.object_parsedef("Pos", [[
    x,y :float
]])

local Actor = StatML.class_parsedef("Actor", [[
    Pos as xy
]])

local World = StatML.object_parsedef("World", [[
    actors :list
]])

function M.main()
    StatML.load_and_parse_file(path_resolve "demoshoot.yaml")
    local actor_set = StatML.get_instance_set "Actor"
    pretty_print(actor_set.list)
end

--------------------------------------------------------------------------------
-- Class implementations
--------------------------------------------------------------------------------

function Actor:on_draw()

end

return M