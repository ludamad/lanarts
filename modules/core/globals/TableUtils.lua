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

function table.clear(t)
    for i=#t,1 do
        t[i] = nil
    end
    for k,v in pairs(t) do
        t[k] = nil
    end
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
    for v in values(t2) do
        t1[#t1 + 1] = v
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


function table.add(a,b)
    for k,v in pairs(b) do
        local bval = b[k]
        if type(bval) == "table" then table.add(a[k], bval)
        else a[k] = (a[k] or 0) + bval end
    end
end