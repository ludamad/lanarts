-- Refactored from http://lua-users.org/wiki/AnsiTerminalColors to be more lanartsy
-- As well, added support for SGR parameters

local M = nilprotect {} -- Submodule

local AnsiColor = newtype()

local function val2string(value, --[[Optional]] sgr_params)
    sgr_params = sgr_params or ""
    return '\27[' .. tostring(value) .. sgr_params .. 'm'
end

function AnsiColor:init(value)
    self.value = value
end

function AnsiColor:__tostring()
    return val2string(self.value)
end

function AnsiColor:__concat(other)
    return tostring(self) .. tostring(other)
end

function AnsiColor:__call(s, --[[Optional]] sgr_params)
    return val2string(self.value, sgr_params) .. s .. M.RESET
end

local color_codes = {
    -- attributes
    RESET = 0,
    CLEAR = 0,
    BRIGHT = 1,
    DIM = 2,
    UNDERSCORE = 4,
    BLINK = 5,
    REVERSE = 7,
    HIDDEN = 8,

    -- foreground
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37,

    -- background
    ON_BLACK = 40,
    ON_RED = 41,
    ON_GREEN = 42,
    ON_YELLOW = 43,
    ON_BLUE = 44,
    ON_MAGENTA = 45,
    ON_CYAN = 46,
    ON_WHITE = 47,
}

for k, v in pairs(color_codes) do
    M[k] = AnsiColor.create(v)
end

M.ansi_color_create = AnsiColor.create

-- For use as 'sqr_params' parameter. Can be chained with concatenaton.
M.BOLD = ";1"
M.FAINT = ";2"
M.ITALIC = ";3"
M.UNDERLINE = ";4"
M.CROSSED_OUT = ";9"
M.DEFAULT = ""

local function rgb_to_xterm(rgb)
    local incs = {0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff}
    local res = {}
    for col in values(rgb) do
        for i= 1, #incs - 1 do
            local s, b = incs[i], incs[i+1] -- smaller, bigger
            if s <= col and col <= b then
                local s1 = math.abs(s - col)
                local b1 = math.abs(b - col)
                table.insert(res, s1 < b1 and s or b)
                break
            end
        end
    end
    local ColorLookupTable = require "terminal.ColorLookupTable"
    local equiv = ColorLookupTable.table[string.format("%.2x%.2x%.2x", unpack(res))]
    return equiv
end

function M.from_rgb(color) 
   return AnsiColor.create("38;5;" .. rgb_to_xterm(color))
end

function M.print(text, col, --[[Optional]] sgr_params)
    io.write(col(text, sgr_params))
    io.flush(io.stdout)
end

function M.println(text, col, --[[Optional]] sgr_params)
    io.write(col(text, sgr_params), '\n')
    io.flush(io.stdout)
end

return M