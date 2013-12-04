local Tmpl = import "@Templates"
local Types = import "@FieldTypes"

function TestCases.test_typedef()
    local t = typedef [[
        x, y : int
    ]]
    local p = t.create(1,2)
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
