local moonc = require("moonscript.cmd.moonc")

local function main(raw_args)
    local src = raw_args[1]
    local dest = raw_args[2]
    if not src or not dest then
        error("Usage: engine.CompileMoonScript <source.moon> <dest.lua>")
    end
    local ok, err = moonc.compile_and_write(src, dest)
    if not ok then
        error(("%s: %s"):format(src, err or "compile failed"))
    end
end

return main
