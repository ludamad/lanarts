local M = nilprotect {} -- Submodule

local function _preinit(T, defs, corrections)
    for f in T:all_subfields() do
        if f.type.children > 0 then 
            append(defs, ("setmetatable({%s}, meta_%s)"):format("false, " .. f.offset, f.typename))
            append(corrections, ("rawget(data, %s)[1] = data"):format(f.offset))
        else append(defs, f.type:emit_default()) end
    end
end

function M.emit_preinit(T, slice)
    local defs, corrections = {}, {}
    _preinit(T, defs, corrections)
    return ("local data,pos = {%s},0\n%s = setmetatable({data,pos}, %s)\n%s"):format(
        (",\n"):join(defs), slice, "METATABLE", ("\n"):join(corrections)
    ):split("\n")
end

function M.emit_field_init(T, S, kroot, args)
    local parts = {} ; for field in values(T.fields) do
        local offset = kroot + field.offset
        append(parts, field.type:emit_init(S, offset))
    end ; return parts
end

local AssignBatcher = newtype()
function AssignBatcher:init(kroot)
    self.kroot = kroot
    self.offset = false ; self.batches = 0 ; self.dx = false
end
function AssignBatcher:add(parts, f)
    if not self.offset then 
        self.offset = f.offset ; self.batches = 1 ; return
    end
    if not self.dx then self.dx = f.offset - self.offset
    else
        local dist = f.offset - self.offset - (self.batches-1) * self.dx
        if self.dx ~= dist then
            self:emit(parts) ; self:add(parts, f) ; return
        end
    end

    self.batches = self.batches + 1
end
function AssignBatcher:emit(parts)
    if not self.offset then return end
    local dx = self.dx or 1
    local i_start, i_end = self.offset, self.offset+(self.batches-1)*dx
    if self.batches <= 3 then
        for i=i_start,i_end,dx do
            append(parts, ("  rawset(data, pos+%d, rawget(odata, opos+%d))"):format(
                self.kroot + i, i
            ))
        end
    else
        append(parts, ("  for i=%d,%d%s do rawset(data, pos+%d+i, rawget(odata, opos+i)) end"):format(
            i_start,i_end, (dx == 1) and '' or ','..self.dx, self.kroot
        ))
    end 
    self:init(self.kroot)
end

function M.emit_field_assign(T, S, kroot, O)
    local parts = {fmt("do local odata, opos = rawget(%s,1),rawget(%s,2) --<%s:emit_field_assign>", O, O, T.name)}
    local batcher = AssignBatcher.create(kroot)
    for f in T:all_leafs() do
        batcher:add(parts, f)
    end
    batcher:emit(parts)
    append(parts, "end")
    return parts
end

return M