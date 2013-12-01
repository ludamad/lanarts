local ResourceTypes = import "unstable.ResourceTypes"

local function assert_no_array_portion(t)
    for i=1,#t do 
        lunit.assert(t[i] == nil)
    end
end

local function define_and_assert_consistency(R, t)
    local entry = R.define(t)

    assert_no_array_portion(entry)

    lunit.assert(entry.name ~= nil)
    lunit.assert(entry.id ~= nil)

    lunit.assert_equal(entry, R.lookup(entry.name))
    lunit.assert_equal(entry, R.lookup(entry.id))

    return entry
end

function TestCases.define_and_lookup()
    local R = ResourceTypes.type_create()
    local TEST_ENTRIES = 10

    for i=1,TEST_ENTRIES do
        local closure_called = false
        local entry = define_and_assert_consistency(R, {
            function(entry)
                closure_called = true
                entry.name = "Foo" .. i
                entry.test_stat = entry.test_stat .. "Baz"
            end,
            test_stat = "Bar"
        })
    
        lunit.assert(closure_called)
        lunit.assert_equal("Foo" .. i, entry.name)
        lunit.assert_equal(i, entry.id)
        lunit.assert_equal("BarBaz", entry.test_stat)
    end

    for i=1,TEST_ENTRIES do
        lunit.assert(R.lookup(i) ~= nil)
        lunit.assert(R.lookup("Foo" .. i) ~= nil)
        lunit.assert_equal(R.lookup(i), R.lookup("Foo" .. i))
    end
end