-- Simple logging levels:

local iowrite = io.write --performance

local M = nilprotect {}

local levels = {}
for i,v in ipairs {"VERBOSE", "INFO", "WARN", "NONE"} do M[v] = v ; levels[v] = i end

function M.set_log_level(log)
	local level = levels[log]
	if not levels[log] then
		error("'"..log.."' is not a valid log level!")
	end
	
	for k,l in pairs(levels) do
	    if k ~= "NONE" then
            local colors = {
            	VERBOSE = '\27[37;2m',
            	INFO = '\27[33;2m',
            	WARN = '\27[31m',
            }
            local reset = '\27[0m'
            local prefix = colors[k] .. '[' .. k:lower() .. '] ' .. reset
            _G['log_' .. k:lower()] = (l < level) and function() end or function(...) 
            	iowrite(prefix, ..., reset, '\n')
            end
		end
	end
        _G.log = _G.log_info
end

if os.getenv("LANARTS_HEADLESS") then
    M.set_log_level "WARN"
else
    M.set_log_level "VERBOSE"
end

return M
