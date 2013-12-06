local AnsiColors -- Lazily imported

local function colorfmt_aux(sub)
    printf("'%s'",sub)
    local col,str = sub:match("^%[([%w_]+)|(.*)%]$")
    if not col then return sub end
    local mod = nil
    local rest = col:match("^BOLD_(.*)$")
    if rest then col = rest ; mod = AnsiColors.BOLD end
    str = AnsiColors[col](str, mod)
    str = str:gsub("%b[]", colorfmt_aux)
    return str
end
function colorfmt(str)
    AnsiColors = AnsiColors or import "@AnsiColors"
    local str = str:gsub("%b[]", colorfmt_aux)
    return str
end
function colprintf(str, ...)
    return print(colorfmt(fmt(str, ...)))
end
