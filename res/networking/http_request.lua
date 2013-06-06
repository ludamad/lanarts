-- Written for Lanarts

-- Primarily for use with luasocket's socket.http & coroutines
-- Reads in line by line

local jsonlib = require "json"
local socket = require "socket"
local http = require "socket.http"
local ltn12 = require "ltn12"

require "utils" -- for table.pop_front & do_nothing

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

--- Calls yield instead of blocking
function http_request(url, message)
    local response_parts = {}
    local reqt = {
        url = url,
        method = "GET",
        create = YieldingSocket.create,
        sink = ltn12.sink.table(response_parts)
    }
    if message then        
        reqt.source = ltn12.source.string(message)
        reqt.headers = {
            ["content-length"] = #message,
            ["content-type"] = "application/x-www-form-urlencoded"
        }
        reqt.method = "POST"
    end
    pretty_print(reqt)
    local code, headers, status = socket.skip(1, http.trequest(reqt))
    return table.concat(response_parts), code, headers, status
end

--- Calls yield instead of blocking
function json_request(url, message)
    -- nil if message is nil
    print "JSON REQUEST"
    local json_request = message and jsonlib.generate(message)
    local json_response, code, headers, status = http_request(url, json_request)
    print "JSON REQUEST"
    local status, response_table = jsonlib.parse(json_response)
    if not status then error(response_table) end

    return response_table, code, headers, status
end