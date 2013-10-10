local M = nilprotect {} -- Submodule

local ADJ1 = {"Glowing", "Shiny", "Chipped", "Slimy", "Cold"}
local ADJ2 = {"Black", "Purple", "Gold", "Silver", "Seethrough"}
function M.random_ring_name()
    return random_choice(ADJ1) .. ' ' .. random_choice(ADJ2) .. ' ' .. 'Ring'
end

function M.random_amulet_name()
    local kind = random_choice{'Amulet', 'Necklace', 'Talisman', 'Charm', 'Periapt'}
    return random_choice(ADJ1) .. ' ' .. random_choice(ADJ2) .. ' ' .. kind
end

return M