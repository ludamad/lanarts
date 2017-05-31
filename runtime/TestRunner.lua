local M = nilprotect {} -- Submodule

--TODO
local function handle_cpp_tests(args)
    if not table.contains(args, "--cpp") then return end

    local failures = _lanarts_unit_tests()
    if failures > 0 then
        printf("'%d' failures in cpp tests!", failures)
    end
end

local DOTLINE = "---------------------------------"


local function pluralize(num, noun)
    return num .. " " .. noun .. (num == 1 and "" or "s")
end

local function run_tests(suitelist)
    local AC = require "terminal.AnsiColors"

    local test_n,fails = 0,0
    local summary = {"{bold_white:Summary:} {white:|}"}
    for _, suite in ipairs(suitelist) do
        for _, test in ipairs(suite) do
            test_n = test_n + 1
            local name,f = unpack(test)
            local ok,err = xpcall(f, debug.traceback)
            if not ok then 
                colprintf("{bold_white:%d)} {bold_red:× FAILURE}{white: for }{bold_white:%s}{white: (in %s)}", test_n, name, suite.name)
                print(err)
                fails = fails + 1
            else
                colprintf("{bold_white:%d)} {bold_white:✓ Pass}{white: for }{bold_white:%s}{white: (in %s)}", test_n, name, suite.name)
            end
            append(summary, ok and "{bold_white:✓}" or "{bold_red:×}")
        end
        append(summary, '{white:|}')
    end

    colprintf("{white:%s}", DOTLINE)
    local fails_str = pluralize(fails, "failure")
    append(summary, fails == 0 and " {white:with no failures.}" or " {white:with }{bold_red:"..fails_str..".}")
    colprintf(table.concat(summary))

    if fails > 0 then
        colprintf("{bold_white:Some tests have} {bold_red:failed!}\n{bold_white:%d/%d} {bold_red:tests failed.}", fails, test_n)
    else
        colprintf("{bold_white:Test suite has passed.\nAll %d tests have} {bold_green:passed!}", test_n)
    end
    colprintf("{white:%s}", DOTLINE)
    return (fails == 0)
end

function M.main(args)
    local Display = require "core.Display"
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
            for _, test in ipairs(find_submodules(module .. ".tests", true)) do 
                import_dofile(test)
            end
        end
    end

    return run_tests(suitelist)
end

return M
