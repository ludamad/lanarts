local M = nilprotect {} -- Submodule

local function handle_cpp_tests(args)
    if not table.contains(args, "--cpp") then return end

    local failures = _lanarts_unit_tests()
    if failures > 0 then
        printf("'%d' failures in cpp tests!", failures)
    end
end

function M.main(args)
    local Display = import "core.Display"
    -- XXX: We must initialize the display context for tests to be able to load images.
    Display.initialize("Tests", {settings.view_width, settings.view_height}, settings.fullscreen)

    local module_filter,test_filter = sysargs.get_varparam("--tests", --[[Max params]] 2)
    module_filter,test_filter = module_filter or ".*", test_filter or ".*"

    import '@lunit'
    local testcases = {}
    
    local function get_testlist(name)
        if not testcases[name] then 
            local fake_module = { _NAME = name, tests = {} }
            lunit.testcase(fake_module)
            testcases[name] = fake_module
        end
        return testcases[name].tests
    end
    -- Magic test table, uses module name to add lunit test-case
    TestCases = setmetatable({}, {
        -- A bit of a hack, but suffices for now
        __newindex = function(__unused, key, val)
            if key:match(test_filter) then
                local test_list = get_testlist(virtual_path(2))
                append(test_list, {key, val})
            end
        end
    })

    assert = lunit.assert

    local tests = {}
    for module,package in module_iter() do
        if module:match(module_filter) then 
            for test in values(find_submodules(module .. ".tests", true)) do 
                append(tests, test)
            end
        end
    end
    local stats = lunit.main{'--', unpack(tests)}

    return (stats.failed == 0 and stats.errors == 0)
end

return M