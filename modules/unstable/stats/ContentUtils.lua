local M = nilprotect {} -- Submodule

-- Derive sprite from name
function M.derive_sprite(name, --[[Optional]] stack_idx)
    stack_idx = (stack_idx or 1) + 1
    local path = path_resolve("sprites/" .. name:gsub(' ', '_'):lower() .. ".png", stack_idx)
    return image_cached_load(path)
end

return M