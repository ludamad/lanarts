local function file_exists(name)
	local f=io.open(name,"r")
	if f~=nil then 
		io.close(f) 
		return true
	end
	return false 
end

local function optional_load(file, loader) 
	if not file_exists(file) then
		return { 
			play = function() end, 
			loop = function() end 
		}
	end
	return loader(file)
end

-- Returns a dummy object if the file doesn't exist
function music_optional_load(file) 
	return optional_load(file, music_load)
end

-- Returns a dummy object if the file doesn't exist
function sound_optional_load(file) 
	return optional_load(file, sound_load)
end

local hitsound = sound_optional_load("res/sound/hit.wav")

function play_hit_sound() 
	--hitsound:play()
end