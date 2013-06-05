-- Written for Lanarts

-- Primarily for use with luasocket's socket.http & coroutines
-- Reads in line by line

require "utils" -- for table.pop_front & do_nothing

local socket = require "socket"

local YieldingSocket = newtype()

-- Intervals to wait for connect before yielding
local CONNECT_INTERVAL = 1000 -- 1000 ms

function YieldingSocket:init(...)
    print ("CALLED!")
    self.socket = socket.tcp(...)
    self.socket:settimeout(0)
end

local function print_ret(fn, ...)
    print(fn .. " returned : ", ...)
    return ...
end

function YieldingSocket:connect(...)
    while true do
        self.socket:settimeout(CONNECT_INTERVAL)
        local status, err = self.socket:connect(...)
        if err == "timeout" then
            coroutine.yield()
        else
            self.socket:settimeout(0)
            return status, err
        end
    end
end

function YieldingSocket:close(...)
    return print_ret("close", self.socket:close(...) )
end

function YieldingSocket:send(...)
    return print_ret("send", self.socket:send(...) )
end

function YieldingSocket:settimeout()
    return print_ret("settimeout", self.socket:settimeout(0) )
end

function YieldingSocket:receive(pattern, ...)
    local line = ''
    while true do
        local result, err, part = print_ret("for receive", self.socket:receive(pattern, ...))
        print (" Got pattern=", pattern, " rest = {", ..., "}, err=", err)
        if not result and err == "timeout" then 
            line = line .. part
            print "YIELD"
            coroutine.yield()
        elseif err ~= nil then
            return nil, err
        else -- We got a 'full' message
            line = line .. result
            return print_ret("receive", line)
        end
    end
end

return YieldingSocket