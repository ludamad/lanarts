local M = nilprotect {} -- Submodule

--TODO
local function handle_cpp_tests(args)
    if not table.contains(args, "--cpp") then return end

    local failures = _lanarts_unit_tests()
    if failures > 0 then
        printf("'%d' failures in cpp tests!", failures)
    end
end

local function run_tests(suitelist)
    local AC = import "terminal.AnsiColors"

    local test_i,fails = 1,0
    for suite in values(suitelist) do
        for test in values(suite) do
            local name,f = unpack(test)
            local ok,err = pcall(f)
            colprintf("[BOLD_RESET|%d) %s][WHITE| for %s (in %s)]", test_i, "Pass", name, suite.name)
            test_i = test_i + 1
        end
    end
end

function M.main(args)
    local Display = import "core.Display"
    -- XXX: We must initialize the display context for tests to be able to load images.
    Display.initialize("Tests", {settings.view_width, settings.view_height}, settings.fullscreen)

    local module_filter,test_filter = sysargs.get_varparam("--tests", --[[Max params]] 2)
    module_filter,test_filter = module_filter or ".*", test_filter or ".*"

    local suitemap, suitelist = {}, {}

    local function ensure_suite(name)
        if not suitemap[name] then -- Previously unseen suite 
            suitemap[name] = {name=name} ; append(suitelist, suitemap[name])
        end
        return suitemap[name]
    end

    -- Magic test table, uses module name to derive suite, and adds to suite test table:
    TestCases = setmetatable({}, {
        __newindex = function(__unused, key, val)
            if key:match(test_filter) then
                local suite = ensure_suite(virtual_path(2))
                append(suite, {key, val})
            end
        end
    })

    -- Run 'import_dofile' on all identified tests:
    for module in module_iter() do
        if module:match(module_filter) then
            import_dofile_if_file(module ..'._Tests')
            for test in values(find_submodules(module .. ".tests", true)) do 
                import_dofile(test)
            end
        end
    end

    return run_tests(suitelist)
end

return M