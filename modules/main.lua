require_path_add("modules/?.lua")

--require "core.main"

-- Hardcoded for now!
local modules = {"lanarts"}

for m in values(modules) do
--    require(m .. ".main")
end
