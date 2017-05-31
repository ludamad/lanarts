function string:interpolate(table)
    return (self:gsub('($%b{})', function(w) return table[w:sub(3, -2)] or w end))
end
function string:trim()
  return self:gsub("^%s*(.-)%s*$", "%1")
end
function string:split(delim)
    local ret = {}
    for m in self:gmatch("([^"..delim.."]+)") do
        ret[#ret + 1] = m
    end
    return ret
end

function string:trimsplit(s)
    local parts = self:split(s)
    for i,p in ipairs(parts) do parts[i] = p:trim() end
    return parts
end

function string:startswith(s)
    return self:sub(1, #s) == s
end
function string:endswith(s)
    return s == '' or self:sub(-#s) == s
end
