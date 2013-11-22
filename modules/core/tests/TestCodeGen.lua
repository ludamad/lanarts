local TypeCompiler = import "@codegen.TypeCompiler"
local TypeSchema = import "@codegen.TypeSchema"
local Types = import "@codegen.Types"

local C = TypeCompiler.create()

function TestCases.type_type_builder()
    local S = TypeSchema.create [[
        a, b : int
        c, d : float
    ]]
    local expected = {a=Types.int,b=Types.int,c=Types.float,d=Types.float}
    for field in values(S.fields) do
        assert(field.name and field.type and (field.is_ref ~= nil))
        assert(expected[field.name] == field.type)
    end
    -- Test __index
    local index_func = loadstring('return ' .. C:index_compile(S))()
    for i, k in ipairs {"a", "b", "c", "d"} do
        local data = {1, 2, 3, 4}
        assert(index_func(data, k) == data[i])
    end
    -- Test __newindex
    local newindex_func = loadstring('return ' .. C:newindex_compile(S))()
    for i, k in ipairs {"a", "b", "c", "d"} do
        local data = {}
        newindex_func(data, k, i)
        assert(data[i] == i)
    end
    -- Test create
    local meta = C:compile(S)
    local val = meta.create(1,2,3,4)
    for i, k in ipairs {"a", "b", "c", "d"} do
        assert(val[k] == rawget(val, i))
    end
    local val = meta.create(0,0,0,0)
    for i, k in ipairs {"a", "b", "c", "d"} do
        assert(val[k] == 0)
        val[k] = i
        assert(val[k] == i)
    end
end

function TestCases.test_typedef()
    local t = typedef [[
        x, y : int
    ]]
    local p = t.create(1,2)
    print(p)
    assert(p.x == 1 and p.y == 2)
    assert(not pcall(function()
        p.x = 1.1 -- Should error!
    end))
    assert(not pcall(function()
        p.y = 1.1 -- Should error!
    end))
    p.x, p.y = 2, 1
    assert(p.x == 2 and p.y == 1)
end