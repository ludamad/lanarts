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
    local test_i = 1
    for suite in values(suitelist) do
        for test in values(suite) do
            local name,f = unpack(test)
            local ok,err = f()
            print(ok,err)
--          1) ERROR for test_typedef (in test_typedef)

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
            suitemap[name] = {} ; append(suitelist, suitemap[name])
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

    -- Run 'dofile' on all identified tests:
    for module,package in module_iter() do
        if module:match(module_filter) then
            local root_test_file = (package..'/'..module..'/_Tests.lua')
            if file_exists(root_test_file) then dofile(root_test_file) end
            for test in values(find_submodules(module .. ".tests", true)) do 
                local rpath = virtual_path_to_real(package, test)
                dofile(rpath)
            end
        end
    end

    return run_tests(suitelist)
end

return M