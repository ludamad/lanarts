function string:interpolate(table)
    return (self:gsub('($%b{})', function(w) return table[w:sub(3, -2)] or w end))
end

function string:trim()
  return self:gsub("^%s*(.-)%s*$", "%1")
end