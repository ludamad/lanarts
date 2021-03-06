EngineInternal = require "core.EngineInternal"
argparse = require "argparse"

main = (raw_args) ->
    parser = argparse("lanarts", "Lanarts shell. Run commands in a Lanarts context.")
    parser\option("-L --lua", "Run lua files.")\count("*")
    -- (1) Parse arguments
    args = parser\parse(raw_args)
    -- (2) Run all lua files/modules
    for target in *args.lua
        modulename = target\gsub(".moon", "")\gsub(".lua", "")\gsub("/", ".")
        _, err = pcall(require, modulename)
        if err
            print "Error while loading '#{modulename}'"
            print err
    -- (3) Free global variables so repl can modify them
    setmetatable(_G, nil)

    -- Optionally, run a Lua file given a normal (ie, not virtual) path
    -- Then drop into a Lua REPL session. The user can exit with 'start_lanarts()'
    while true
        EngineInternal.read_eval_print()

return main
