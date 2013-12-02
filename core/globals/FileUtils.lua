local Sound = import "core.Sound"

--- Return a dummy sound or music object if the given file does not exist.
-- Otherwise return the result of 'loader' on the file.
local function snd_optional_load(file, loader)  
    if not file_exists(file) then 
        return {  
            play = function() end,  
            loop = function() end  
        } 
    end 
    return loader(file) 
end

--- Returns a dummy object if the file doesn't exist
function music_optional_load(file) 
    return snd_optional_load(file, Sound.music_load)
end

--- Returns a dummy object if the file doesn't exist
function sound_optional_load(file) 
    return snd_optional_load(file, Sound.sound_load)
end

function file_as_string(name)
    local f = io.open(name,"r")
    if f == nil then return nil end
    local contents = f:read("*all")
    f:close()
    return contents
end