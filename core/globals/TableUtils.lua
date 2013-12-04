-- TODO: If such monolithic extension of the 'table' package is to be considered kosher, we must document this fully.

-- Cache for small performance boost
local type, select, setmetatable, getmetatable, rawget, pairs, ipairs, table = type, select, setmetatable, getmetatable, rawget, pairs, ipairs, table

function table.merge(t1, t2)
    for k,v in pairs(t2) do t1[k] = v end
end

local function metacopy(t1, t2)
    local meta = getmetatable(t1)
    if meta then
        local copy = meta.__copy
        if copy then
            copy(t1, t2)
            return true
        end
    end
    return false
end

-- Copies 'plain-old arrays' deeply.
function table.deep_array_copy(t1, t2)
    for i=1,#t1 do 
        t2[i] = t1[i] end
    for i=#t2,#t1-1,-1 do -- Reverse for-loop to remove any excess
        t2[i] = nil
    end
end

function table.copy(t1, t2, invoke_meta)
    if invoke_meta == nil then invoke_meta = true end
    setmetatable(t2, getmetatable(t1))
    if invoke_meta and metacopy(t1, t2) then
        return
    end

    for k,_ in pairs(t2) do
        t2[k] = nil
    end
    for k,v in pairs(t1) do
        t2[k] = v
    end
end

function table.deep_copy(t1, t2, invoke_meta)
    if invoke_meta == nil then invoke_meta = true end
    setmetatable(t2, getmetatable(t1))
    if invoke_meta and metacopy(t1, t2) then
        return
    end

    for k,_ in pairs(t2) do
        if rawget(t1, k) == nil then
            t2[k] = nil
        end
    end
    for k,v in pairs(t1) do
        if type(v) == "table" then
            local old_val = rawget(t2, k)
            if old_val then
                table.deep_copy(v, old_val)
            else
                t2[k] = table.deep_clone(v)
            end
        else
            t2[k] = v
        end
    end
end

function table.clone(t)
    local meta = getmetatable(t)
    if meta then
        local clone = meta.__clone
        if clone then return clone(t) end
    end
    local newt = {}
    table.copy(t, newt)
    return newt
end

function table.deep_clone(t)
    local meta = getmetatable(t)
    if meta then
        local clone = meta.__clone
        if clone then return clone(t) end
    end
    local newt = {}
    table.deep_copy(t, newt)
    return newt
end

function table.assign(t, ...)
    table.clear(t)
    for index=1,select("#", ...) do
        t[index] = select(index, ...)
    end
end

function table.key_list(t)
    local ret = {}
    for k,_ in pairs(t) do
        table.insert(ret,k)
    end
    return ret
end

function table.sorted_key_list(t, ...)
    local ret = table.key_list(t)
    table.sort(ret, ...)
    return ret
end

function table.value_list(t)
    local ret = {}
    for _,v in pairs(t) do
        table.insert(ret,v)
    end
    return ret
end

function table.value_key_invert(t)
    local ret = {}
    for k,v in pairs(t) do
        ret[v] = k
    end
    return ret
end

function table.filter_matches(t, pattern)
    for i=#t,1,-1 do
        if t[i]:match(pattern) then
            t[i] = t[i - 1]
        end
    end
end

-- Trick to get internal 'next' function
local next,_ = pairs({})
_G.next = next
function table.is_empty(t)
    return #t==0 and not next(t)
end

function table.pop_first(t)
    local len = #t
    if len == 0 then 
        return nil 
    end
    local val = t[len]
    for i=1,len-1 do
        t[i] = t[i+1]
    end
    t[len] = nil
    return val
end

function table.pop_last(t)
    local val = t[#t]
    t[#t] = nil
    return val    
end

function table.contains(t, val)
    for i=1,#t do 
        if t[i] == val then 
            return true
        end
    end
    return false
end

--- NOTE: Operates on array portion of table
-- Removes all occurences of 'val' from the table, compacting it
function table.remove_occurrences(t, val)
    local new_len = 1
    -- Compact array
    for i = 1, #t do
        if t[i] ~= val then
            t[new_len] = t[i]
            new_len = new_len + 1
        end
    end
    for i = new_len, #t do
        t[i] = nil -- delete values at end
    end
end

function table.insert_all(t1, t2)
    for _, v in ipairs(t2) do
        table.insert(t1, v)
    end
end

--- Adds values from src to dest, copying them if initially nil
function table.defaulted_addition(src, dest)
    for k,v in pairs(src) do
        local val = dest[k]
        if dest[k] == nil then
            dest[k] = src[k]
        else
            if type(val) == 'table' then
                table.defaulted_addition(src[k], val)
            else
                dest[k] = val + src[k]
            end
        end
    end
end

--- Subtracts values from src to dest, copying them if initially nil
function table.defaulted_subtraction(src, dest)
    for k,v in pairs(src) do
        local val = dest[k]
        if dest[k] == nil then
            dest[k] = -src[k]
        else
            if type(val) == 'table' then
                table.defaulted_subtraction(src[k], val)
            else
                dest[k] = val - src[k]
            end
        end
    end
end

function table.scaled(t, scale)
    local ret = {}
    for k,v in pairs(t) do
        ret[k] = v*scale
    end
    return ret
end

function table.slice(t, i_start, i_end)
    local ret = {}
    for i=i_start,i_end do
        append(ret, t[i])
    end
    return ret
end

function table.add(a,b)
    for k,v in pairs(b) do
        local bval = b[k]
        if type(bval) == "table" then table.add(a[k], bval)
        else a[k] = (a[k] or 0) + bval end
    end
end