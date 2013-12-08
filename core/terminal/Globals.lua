local AnsiColors -- Lazily imported

local PARAM_SPECS = {
    BOLD = ";1",
    FAINT = ";2",
    ITALIC = ";3",
    UNDERLINE = ";4",
    CROSS = ";9",
}

local function resolve_col_and_params(colspec)
    -- Split comma-separated traits
    local traits = colspec:upper():split("_")
    local col,params = "RESET",""
    for t in values(traits) do
        if PARAM_SPECS[t] then params = params .. PARAM_SPECS[t] 
        elseif t ~= '' then col = t end
    end
    return col,params
end

local function colorfmt_aux(sub)
    local colspec,str = sub:match("^%{([%w_]+):(.*)%}$")
    if not colspec then return sub end
    local col, params = resolve_col_and_params(colspec)
    str = AnsiColors[col](str, params)
    str = str:gsub("%b{}", colorfmt_aux)
    return str
end
function colfmt(str, ...)
    AnsiColors = AnsiColors or import "@AnsiColors"
    local str = str:gsub("%b{}", colorfmt_aux):format(...) ; return str
end
function colprintf(str, ...)
    return print(colfmt(str, ...))
end
