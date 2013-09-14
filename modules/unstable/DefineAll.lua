-- Import all content definition submodules.
local CONTENT_PATTERN = "Define*"
local content = find_submodules("unstable", --[[Recursive]] true, CONTENT_PATTERN)

for c in values(content) do
    if c ~= "unstable.DefineAll" then
        -- Don't recursively import!
        import(c)
    end
end