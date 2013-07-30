local M = {} -- submodule

--- Adds values from src to dest, copying them if initially nil
function M.defaulted_add(src, dest)
    for k,v in pairs(src) do
        local val = dest[k]
        if dest[k] == nil then
            dest[k] = src[k]
        else
            dest[k] = val + src[k]
        end
    end
end

return M
