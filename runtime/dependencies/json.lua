---
-- Modified for Lanarts. Took out unicode which depended on lua-jit.
-- TODO: Replace this with a better JSON implementation once performance is bad or unicode is needed.
-- 
-- Library methods for handling JSON data. It handles JSON encoding and
-- decoding according to RFC 4627.
--
-- There is a test section at the bottom which shows some example
-- parsing. If you want to parse JSON, you can test it by pasting sample JSON
-- into the <code>TESTS</code> table and run the <code>test</code> method
--
-- There is a straightforward mapping between JSON and Lua data types. One
-- exception is JSON <code>NULL</code>, which is not the same as Lua
-- <code>nil</code>. (A better match for Lua <code>nil</code> is JavaScript
-- <code>undefined</code>.) <code>NULL</code> values in JSON are represented by
-- the special value <code>json.NULL</code>.
--
-- @author Martin Holst Swende
-- @copyright Same as Nmap--See http://nmap.org/book/man-legal.html

-- TODO: Unescape/escape unicode
-- Version 0.4
-- Created 01/25/2010 - v0.1 - created by Martin Holst Swende <martin@swende.se>
-- Heavily modified 02/22/2010 - v0.3. Rewrote the parser into an OO-form, to not have to handle
-- all kinds of state with parameters and return values.
-- Modified 02/27/2010 - v0.4 Added unicode handling (written by David Fifield). Renamed toJson
-- and fromJson intogenerate() and parse(), implemented more proper numeric parsing and added some more error checking.

module("json", package.seeall)

-- Javascript null representation, see explanation above
NULL = {}

-- See section 2.5 for escapes.
-- For convenience, ESCAPE_TABLE maps to escape sequences complete with
-- backslash, and REVERSE_ESCAPE_TABLE maps from single escape characters
-- (no backslash).
local ESCAPE_TABLE = {}
local REVERSE_ESCAPE_TABLE = {}
do
    local escapes = {
        [string.char(0x22)] = "\"",
        [string.char(0x5C)] = "\\",
        [string.char(0x2F)] = "/",
        [string.char(0x08)] = "b",
        [string.char(0x0C)] = "f",
        [string.char(0x0A)] = "n",
        [string.char(0x0D)] = "r",
        [string.char(0x09)] = "t",
    }
    for k, v in pairs(escapes) do
        ESCAPE_TABLE[k] = "\\" .. v
        REVERSE_ESCAPE_TABLE[v] = k
    end
end

-- Escapes a string
--@param str the string
--@return a string where the special chars have been escaped
local function escape(str)
    return "\"" .. string.gsub(str, ".", ESCAPE_TABLE) .. "\""
end

--- Makes a table be treated as a JSON Array when generating JSON
--  A table treated as an Array has all non-number indices ignored.
-- () param t a table to be treated as an array
function make_array(t)
    local mt = getmetatable(t) or {}
    mt["json"] = "array"
    setmetatable(t, mt)
end

--- Makes a table be treated as a JSON Object when generating JSON
--  A table treated as an Object has all non-number indices ignored.
-- () param t a table to be treated as an object
function make_object(t)
    local mt = getmetatable(t) or {}
    mt["json"] = "object"
    setmetatable(t, mt)
end

--- Checks what JSON type a variable will be treated as when generating JSON
-- () param var a variable to inspect
-- () return a string containing the JSON type. Valid values are "array",
--        "object", "number", "string", "boolean", and "null"
function typeof(var)
    local t = type(var)
    if var == NULL then
        return "null"
    elseif t == "table" then
        local mtval = rawget(getmetatable(var) or {}, "json")
        if mtval == "array" or (mtval ~= "object" and #var > 0) then
            return "array"
        else
            return "object"
        end
    else
        return t
    end
    error("Unknown data type in typeof")
end

--- Creates json data from an object
--@param obj a table containing data
--@return a string containing valid json
function generate(obj)

    -- NULL-check must be performed before
    -- checking type == table, since the NULL-object
    -- is a table
    if obj == NULL then
        return "null"
    elseif obj == false then
        return "false"
    elseif obj == true then
        return "true"
    elseif type(obj) == "number" then
        return string.format("%g", obj)
    elseif type(obj) == "string" then
        return escape(obj)
    elseif type(obj) == "table" then
        local k, v, elems, jtype
        elems = {}
        jtype = typeof(obj)
        if jtype == "array" then
            for _, v in ipairs(obj) do
                elems[#elems + 1] = generate(v)
            end
            return "[" .. table.concat(elems, ", ") .. "]"
        elseif jtype == "object" then
            for k, v in pairs(obj) do
                elems[#elems + 1] = escape(k) .. ": " .. generate(v)
            end
            return "{" .. table.concat(elems, ", ") .. "}"
        end
    end
    error("Unknown data type in generate")
end

-- This is the parser, implemented in OO-form to deal with state better
local Json = {}
-- Constructor
function Json:new(input)
    local o = {}
    setmetatable(o, self)
    self.__index = self
    o.input = input
    o.pos = 1 -- Pos is where the NEXT letter will be read
    return o
end

-- Gets next character and ups the position
--@return next character
function Json:next()
    self.pos = self.pos+1
    return self.input:sub(self.pos-1, self.pos-1)
end
-- Updates the position to next non whitespace position
function Json:eatWhiteSpace()
    --Find next non-white char
    local a,b = self.input:find("%S",self.pos)
    if not a then
        self:syntaxerror("Empty data")
        return
    end
    self.pos = a
end

-- Jumps to a specified position
--@param position where to go
function Json:jumpTo(position)
    self.pos = position
end

-- Returns next character, but without upping position
--@return next character
function Json:peek()
    return self.input:sub(self.pos, self.pos)
end

--@return true if more input is in store
function Json:hasMore()
    return self.input:len() >= self.pos
end

-- Checks that the following input is equal to a string
-- and updates position so next char will be after that string
-- If false, triggers a syntax error
--@param str the string to test
function Json:assertStr(str)
    local content = self.input:sub(self.pos,self.pos+str:len()-1)
    if(content == str) then-- All ok
        -- Jump forward
        self:jumpTo(self.pos+str:len())
        return
    end
    self:syntaxerror(("Expected '%s' but got '%s'"):format( str, content))
end

-- Trigger a syntax error
function Json:syntaxerror(reason)
    self.error = ("Syntax error near pos %d: %s input: %s"):format( self.pos, reason, self.input)
end

-- Check if any errors has occurred
function Json:errors()
    return self.error ~= nil
end
-- Parses a top-level JSON structure (object or array).
--@return the parsed object or puts error messages in self.error
function Json:parseStart()
    -- The top level of JSON only allows an object or an array. Only inside
    -- of the outermost container can other types appear.
    self:eatWhiteSpace()
    local c = self:peek()
    if c == '{' then
        return self:parseObject()
    elseif c == '[' then
        return self:parseArray()
    else
        self:syntaxerror(("JSON must start with object or array (started with %s)"):format(c))
        return
    end
end

-- Parses a value
--@return the parsed value
function Json:parseValue()
    self:eatWhiteSpace()
    local c = self:peek()

    local value
    if c == '{' then
        value = self:parseObject()
    elseif c == '[' then
        value = self:parseArray()
    elseif c == '"' then
        value = self:parseString()
    elseif c == 'n' then
        self:assertStr("null")
        value = NULL
    elseif c == 't' then
        self:assertStr("true")
        value = true
    elseif c == 'f' then
        self:assertStr("false")
        value = false
    else -- numeric
        -- number = [ minus ] int [ frac ] [ exp ]
        local a,b =self.input:find("-?%d+%.?%d*[eE]?[+-]?%d*", self.pos)
        if not a or not b then
            self:syntaxerror("Error 1 parsing numeric value")
            return
        end
        value = tonumber(self.input:sub(a,b))
        if(value == nil) then
            self:syntaxerror("Error 2 parsing numeric value")
            return
        end
        self:jumpTo(b+1)
    end
    return value
end
-- Parses a json object {}
--@return the object (or triggers a syntax error)
function Json:parseObject()
    local object  = {}
    make_object(object)
    local _= self:next() -- Eat {

    while(self:hasMore() and not self:errors()) do
        self:eatWhiteSpace()
        local c = self:peek()
        if(c == '}') then -- Empty object, probably
            self:next() -- Eat it
            return object
        end

        if(c ~= '"') then
            self:syntaxerror(("Expected '\"', got '%s'"):format(c))
            return
        end

        local key = self:parseString()
        if self:errors() then
            return
        end
        self:eatWhiteSpace()
        c = self:next()
        if(c ~= ':') then
            self:syntaxerror("Expected ':' got "..c)
            return
        end
        local value = self:parseValue()

        if self:errors() then
            return
        end

        object[key] = value

        self:eatWhiteSpace()
        c = self:next()
        -- Valid now is , or }
        if(c == '}') then
            return object
        end
        if(c ~= ',') then
            self:syntaxerror("Expected ',' or '}', got "..c)
            return
        end
    end
end
-- Parses a json array [] or triggers a syntax error
--@return the array object
function Json:parseArray()
    local array  = {}
    make_array(array)
    self:next()
    while(self:hasMore() and not self:errors()) do
        self:eatWhiteSpace()
        if(self:peek() == ']') then -- Empty array, probably
            self:next()
            break
        end
        local value = self:parseValue()
        if self:errors() then
            return
        end
        table.insert(array, value)
        self:eatWhiteSpace()
        local c = self:next()
        -- Valid now is , or ]
        if(c == ']') then return array end
        if(c ~= ',') then
            self:syntaxerror(("Expected ',' but got '%s'"):format(c))
            return
        end
    end
    return array
end

-- Parses a json string
-- @return the string or triggers syntax error
function Json:parseString()

    local val = ''
    local c = self:next()
    assert( c == '"')
    while(self:hasMore()) do
        local c  = self:next()

        if(c == '"') then -- end of string
            break
        elseif(c == '\\') then-- Escaped char
            local d = self:next()
            if REVERSE_ESCAPE_TABLE[d] ~= nil then
                val = val .. REVERSE_ESCAPE_TABLE[d]
            else
                self:syntaxerror(("Undefined escape character '%s'"):format(d))
                return false
            end
        else -- Char
            val = val .. c
        end
    end
    return val
end
--- Parses json data into an object form
-- This is the method you probably want to use if you
-- use this library from a script.
--@param data a json string
--@return status true if ok, false if bad
--@return an object representing the json, or error message
function parse(data)
    local parser = Json:new(data)
    local result = parser:parseStart()
    if(parser.error) then
        return false, parser.error
    end
    return true, result
end

----------------------------------------------------------------------------------
-- Test-code for debugging purposes below
----------------------------------------------------------------------------------

local TESTS = {
    '{"a":1}',
    '{"a":true}',
    '{"a":     false}',
    '{"a":     null         \r\n, \t "b"    \f:"ehlo"}',
    '{"a\\"a":"a\\"b\\"c\\"d"}',
    '{"foo":"gaz\\"onk", "pi":3.14159,"hello":{ "wo":"rld"}}',
    '{"a":1, "b":2}',
    '{"foo":"gazonk", "pi":3.14159,"hello":{ "wo":"rl\\td"}}',
    '[1,2,3,4,5,null,false,true,"\195\164\195\165\195\182\195\177","bar"]',
    '[]',-- This will yield {} in toJson, since in lua there is only one basic datatype - and no difference when empty
    '{}',

    '',          -- error
    'null',         -- error
    '"abc"',        -- error
    '{a":1}',       -- error
    '{"a" bad :1}',     -- error
    '["a\\\\t"]',       -- Should become Lua {"a\\t"}
    '[0.0.0]',  -- error
    '[-1]',
    '[-1.123e-2]',
    '[5e3]',
    '[5e+3]',
    '[5E-3]',
    '[5.5e3]',
    '["a\\\\"]',        -- Should become Lua {"a\\"}
    '{"a}": 1}',        -- Should become Lua {"a}" = 1}
    '["key": "value"]', -- error
}
function test()
    print("Tests running")
    local i,v,res,status
    for i,v in pairs(TESTS) do
        print("----------------------------")
        print(v)
        status,res = parse(v)
        if not status then print( res) end
        if(status) then
            print(generate(res))
        else
            print("Error:".. res)
        end
    end
end

return getfenv()
