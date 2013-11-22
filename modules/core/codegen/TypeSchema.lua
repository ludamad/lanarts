local Types = import ".Types"

local Field = newtype()
function Field:init(name, type, is_ref, is_embedded)
    self.name, self.type, self.is_ref, self.is_embedded = name, type, is_ref, is_embedded
end
function Field:assign_body(slot, other)
    return ("rawset(self, %d, %s)"):format(slot, other)
end
function Field:index_body(slot)
    return ('k == "%s" then return rawget(self, %d)'):format(self.name, slot)
end
function Field:typecheck_body(val)
    if not self.type.typecheck then return nil end
    return self.type.typecheck:interpolate { self = val}
end
function Field:newindex_body(slot)
    local typecheck = self:typecheck_body("v") 
    local typecheck_str = typecheck and 'assert('..typecheck..',k) ; ' or ' '
    return ('k == "%s" then %s%s'):format(self.name, typecheck_str, self:assign_body(slot, "v"))
end

local TypeSchema = newtype()
function TypeSchema:init(fields) 
    if type(fields) == "string" then 
        self.fields = {}
        self:parse_add(fields) 
    else self.fields = fields end 
end
function TypeSchema:add(field) append(self.fields, field) end

-- Parse & add to schema.
function TypeSchema:parse_add(str)
    local fields = {}
    local lines = str:split("\n")
    for i,line in ipairs(lines) do
        if line:match("^%s*[%w_,%s]+:%s*%w+%s*") then
            local parts = line:split(":")
            local names = parts[1]:split(",")
            local typename = parts[2]:trim()
            if typename == "*" then typename = "any" end
            local type = assert(Types[typename], "Invalid type: " .. typename)
            for name in values(names) do
                name = name:trim()
                assert(name:match("^[%w_]+$"), "'" .. name .. "' is not a valid name.")
                append(fields, Field.create(name, type, false))
            end
        elseif line:match("^%w*$") then
            error("Unexpected line " .. i .. " in type_parse: '" .. line .. "'")
        end
    end
    for f in values(fields) do self:add(f) end
end

return TypeSchema