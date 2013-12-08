-- Simple logging levels:

local iowrite = io.write --performance

local M = nilprotect {}

local levels = {}
for i,v in ipairs {"VERBOSE", "DEBUG", "INFO", "NONE"} do M[v] = v ; levels[v] = i end

local colors = {
	VERBOSE = '\27[37;2',
	DEBUG = '\27[37;2',
	INFO = '\27[37;2',
	NONE = '\27[37;2'
}

function M.set_log_level(log)
	local level = levels[log]
	if not levels[log] then
		error("'"..log.."' is not a valid log level!")
	end
	
	for k,l in pairs(levels) do
		local color = colors[k]
		_G['log_' .. k:lower()] = (l < level) and do_nothing or function(...) 
			iowrite(color, ..., 'm')
		end
	end
end

M.set_log_level "VERBOSE"

log_verbose("TEST")

return M
