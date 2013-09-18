-- Import all content definition submodules.
local CONTENT_PATTERN = "Define*"

local function import_all(subpackage)
    local content = find_submodules(subpackage, --[[Recursive]] true, CONTENT_PATTERN)
    for c in values(content) do
        if c ~= "unstable.DefineAll" then
            -- Don't recursively import!
            import(c)
        end
    end
end

-- Import stats folder first, has fundamental components:
import_all("unstable.stats")
import_all("unstable")