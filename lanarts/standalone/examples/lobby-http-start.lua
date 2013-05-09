require_path_add("res/library_files/json/?.lua")

require "utils" -- for pretty_print

local json = require "json"
local http = require "socket.http"

local conf = {
    url = "http://localhost:8080/"
}

local login_info = {
    username = nil,
    sessionId = nil
}

local function send_request(r)
    local response, status = http.request(conf.url, json.generate(r))
    if status ~= 200 then error(response) end
    return response
end

--instance JSON.ToJSON Message where
--     toJSON (LoginMessage u p) = JSON.object ["type" .= ("LoginMessage" :: T.Text), "username" .= u, "password" .= p]
--     toJSON (GuestLoginMessage u) = JSON.object ["type" .= ("GuestLoginMessage" :: T.Text), "username" .= u]
--     toJSON (CreateUserMessage u p) = JSON.object ["type" .= ("CreateUserMessage" :: T.Text), "username" .= u, "password" .= p]
--     toJSON (ChatMessage u sid msg) = JSON.object ["type" .= ("ChatMessage" :: T.Text), "username" .= u, "sessionId" .= sid, "message" .= msg]
--     toJSON (LoginSuccessMessage sid) = JSON.object ["type" .= ("LoginSuccessMessage" :: T.Text), "sessionId" .= sid]
--     toJSON (ServerMessage cntxt msg) = JSON.object ["type" .= ("ServerMessage" :: T.Text), "context" .= cntxt, "message" .= msg]

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
    end

    return nil
end

local function handle_response(str)
    local status, msg = json.parse(str)
    if not status then 
        error(msg) 
    elseif msg.type == "LoginSuccessMessage" then
        login_info.sessionId = msg.sessionId    
    else 
        pretty_print(msg)
    end
end

local function handle_request(line) 
    local request = parse_request(line)
    if request ~= nil then
        local response = send_request(request) 
        handle_response(response)
    else 
        print "Your request was invalidly formatted!"
    end
end

handle_request("send 1 1")

function main()
    while true do
        local line = io.read()
        if line == nil then break end
        handle_request(line)
        io.flush()
    end
end

