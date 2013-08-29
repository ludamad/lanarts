import 'core.utils'
import '@lunit'

local tests = {
    "tests.unstable.TestStats",
    "tests.unstable.TestResourceTypes",
    "tests.unstable.TestSkillsContent",
    "tests.unstable.TestSpellsContent",
    "tests.unstable.TestSchemas",
    "tests.lanarts.TestObjectRelations",
    "tests.core.TestSerializeBuffer",
    "tests.core.TestGameMap"
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

lunit.main{'--', unpack(tests)}