local M = nilprotect {} -- Submodule

function M.main(args)
    import '@lunit'
    local testcases = {}
    
    local function get_lunit_testcase(name)
        if not testcases[name] then 
            local fake_module = { _NAME = name }
            lunit.testcase(fake_module)
            testcases[name] = fake_module
        end
        return testcases[name]
    end
    
    -- Magic test table, uses module name to add lunit test-case
    TestCases = setmetatable({}, {
        -- A bit of a hack, but suffices for now
        __newindex = function(__unused, key, val)
            get_lunit_testcase(virtual_path(2))[key] = val
        end
    })

    assert = lunit.assert

    local tests = {}
    for module in values {"core", "dungenerate"} do
        if args[2] == nil or args[2] == module then 
            for test in values(find_submodules(module .. ".tests", true)) do 
                append(tests, test)
            end
        end
    end
    local stats = lunit.main{'--', unpack(tests)}

    return (stats.failed == 0 and stats.errors == 0)
end

return M