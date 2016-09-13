local PortalSet = newtype()

function PortalSet:init()
	self.entrances = {}
end

local function ensure_key(self, key)
	self.entrances[key] = self.entrances[key] or {}
end

function PortalSet:set_end(key, val)
	ensure_key(self, key)
	self.entrances[key][2] = val
end

function PortalSet:set_start(key, val)
	ensure_key(self, key)
	self.entrances[key][1] = val
end

function PortalSet:get_end(key)
	return self.entrances[key][2]
end

function PortalSet:get_start(key)
	return self.entrances[key][1]
end

function PortalSet.get:size()
    return #self.entrances
end

return PortalSet