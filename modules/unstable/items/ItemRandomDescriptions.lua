local M = nilprotect {} -- Submodule

local ADJ1 = {"Glowing", "Shiny", "Chipped", "Slimy", "Cold"}
local ADJ2 = {"Black", "Purple", "Gold", "Silver", "Seethrough"}
function M.random_ring_name()
    return random_choice(ADJ1) .. ' ' .. random_choice(ADJ2) .. ' ' .. 'Ring'
end

return M