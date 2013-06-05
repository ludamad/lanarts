local json = require "json"
local socket = require "socket"
local http = require "socket.http"
local ltn12 = require "ltn12"

require "utils"

local YieldingSocket = require "networking.YieldingSocket"

--- Global data. Alter this to change where the lobby points.
lobby_settings = {
    url = "http://localhost:8080/"
}

--- Represents an asynchronous request to the lobby server
-- Implemented using coroutines.
LobbyRequest = newtype()

--- Yielding request. Wrapping this in a coroutine allows us to easily
-- poll by resuming the coroutine.
local function yielding_post_request(url, message)
    local response_parts = {}
    local reqt = {
        url = url,
        method = "POST",
        create = YieldingSocket.create,
        sink = ltn12.sink.table(response_parts),
        source = ltn12.source.string(message),
        headers = {
            ["content-length"] = #message,
            ["content-type"] = "application/x-www-form-urlencoded"
        }
    }
    pretty_print(reqt)
    local code, headers, status = socket.skip(1, http.trequest(reqt))
    return table.concat(response_parts), code, headers, status
end

function LobbyRequest:init(request, handler)
    local json_string = json.generate(request)
    self.request_coroutine = coroutine.create(function() 
        handler(yielding_post_request(lobby_settings.url, json_string))
    end)
end

function LobbyRequest:poll()
    -- We are done when resume returns 'nil'
    return coroutine.resume(thread) == nil
end