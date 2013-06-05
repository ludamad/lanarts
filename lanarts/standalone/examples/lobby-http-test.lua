require "utils" -- for pretty_print
YieldingSocket = require "networking.YieldingSocket"

local json = require "json"
local socket = require "socket"
local http = require "socket.http"
local ltn12 = require "ltn12"

local conf = {
    url = "http://localhost:8080/",
}

local login_info = {
    username = nil,
    sessionId = nil
}

local function yielding_request(url, message)
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

local function send_request(r)
    local json_string = json.generate(r)
    local response, status = yielding_request(conf.url, json.generate(r))
    if status ~= 200 then error(response) end
    return response
end

local function parse_request(line) 
    local parts = line:split(" ")
    local type = parts[1]
    if type == "create" then
        login_info.username = parts[2]
        return { type = "CreateUserMessage", username = login_info.username, password = parts[3] }
    elseif type == "login" then
        login_info.username = parts[2]
        return { type = "LoginMessage", username = login_info.username, password = parts[3] }
    elseif type == "guest" then
        login_info.username = parts[2]
        return { type = "GuestLoginMessage", username = login_info.username}
    elseif type == "send" then
        return { type = "ChatMessage", username = login_info.username, sessionId = login_info.sessionId, message = parts[2] }
    elseif type == "game" then
        return { type = "CreateGameMessage", username = login_info.username, sessionId = login_info.sessionId }
    elseif type == "status" then
        return { type = "GameStatusRequestMessage", gameId = parts[2] }    
    elseif type == "list" then
        return { type = "GameListRequestMessage" }
    end

    return nil
end

local function handle_response(str)
    local status, msg = json.parse(str)
    if not status then 
        error(msg) 
    elseif msg.type == "LoginSuccessMessage" then
        login_info.sessionId = msg.sessionId    
    end 
    pretty_print(msg)
end

local function handle_next_request(line) 
    local request = parse_request(line)
    if request ~= nil then
        local thread = coroutine.create(function()
            local response = send_request(request) 
            handle_response(response)
        end)
        while coroutine.resume(thread) == true do end
    else 
        print "Your request was invalidly formatted!"
    end
end

function main()
    while true do
        local line = io.read()
        if line == nil then break end
        handle_next_request(line)
        io.flush()
    end
end

