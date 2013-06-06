require "networking.http_request"

--- Lobby HTTP requests. They are all nonblocking. Instead, they call yield.
-- Valid only within coroutines.

Lobby = {}

function Lobby.create_user(username, password)
    return json_request(settings.lobby_server_url, {
        type = "CreateUserMessage", 
        username = username, 
        password = password 
    })
end

function Lobby.login(username, password)
    return json_request(settings.lobby_server_url, {
        type = "LoginMessage", 
        username = username, 
        password = password 
    })
end

function Lobby.guest_login(username, password)
    return json_request(settings.lobby_server_url, {
        type = "GuestLoginMessage", 
        username = username
    })
end

function Lobby.chat_message(username, sessionId, message)
    return json_request(settings.lobby_server_url, {
        type = "ChatMessage", 
        username = username,
        sessionId = sessionId,
        message = message
    })
end

function Lobby.create_game(username, sessionId)
    return json_request(settings.lobby_server_url, {
        type = "CreateGameMessage", 
        username = username,
        sessionId = sessionId,
    })
end

function Lobby.query_game(gameId)
    return json_request(settings.lobby_server_url, {
        type = "GameStatusRequestMessage", 
        gameId = gameId
    })
end

function Lobby.query_game_list()
    return json_request(settings.lobby_server_url, {
        type = "GameListRequestMessage" 
    })
end
