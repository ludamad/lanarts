import 'core.utils'
import '@lunit'

local tests = {
    "tests.lanarts.test_stats",
    "tests.lanarts.test_object_relations",
    "tests.core.test_SerializeBuffer"
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

lunit.main{'--', unpack(tests)}