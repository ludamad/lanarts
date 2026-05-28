local EngineInternal = require("core.EngineInternal")
local argparse = require("argparse")
local main
main = function(raw_args)
  local parser = argparse("lanarts", "Lanarts shell. Run commands in a Lanarts context.")
  parser:option("-L --lua", "Run lua files."):count("*")
  local args = parser:parse(raw_args)
  local _list_0 = args.lua
  for _index_0 = 1, #_list_0 do
    local target = _list_0[_index_0]
    local modulename = target:gsub(".moon", ""):gsub(".lua", ""):gsub("/", ".")
    local _, err = pcall(require, modulename)
    if err then
      print("Error while loading '" .. tostring(modulename) .. "'")
      print(err)
    end
  end
  setmetatable(_G, nil)
  while true do
    EngineInternal.read_eval_print()
  end
end
return main
