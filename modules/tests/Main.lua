import "core.Utils"
import '@lunit'

local EXEMPT = {
    "tests.lunit-console",
    "tests.lunit",
    "tests.Main"
}

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
for test in values(find_submodules("tests", true)) do
    if not table.contains(EXEMPT, test) then
        table.insert(tests, test)
    end
end

local stats = lunit.main{'--', unpack(tests)}

return (stats.failed == 0 and stats.errors == 0)