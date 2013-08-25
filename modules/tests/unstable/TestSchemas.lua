local Schemas = import "unstable.Schemas"

local real_assert = assert

local function passes_schema(schema, t)
    local passes = true
    function assert(cond, msg)
        if not cond then
            passes = false
        end
    end

    Schemas.check(schema, t)

    assert = real_assert
    return passes
end 

function TestCases.test_schemas()
    local TEST_TYPE = newtype()

    local schema = {
        foo = Schemas.allow_nil {
            b = Schemas.NOT_NIL,
            c = Schemas.NIL
        },

        bar = Schemas.allow_nil {
            d = Schemas.TABLE,
            e = Schemas.NUMBER,
            f = Schemas.STRING,
            g = Schemas.FUNCTION
        }, 

        baz = Schemas.allow_nil {
            h = Schemas.TABLE_OR_NIL,
            i = Schemas.NUMBER_OR_NIL,
            j = Schemas.STRING_OR_NIL,
            k = Schemas.FUNCTION_OR_NIL
        },

        flibby = Schemas.allow_nil( Schemas.type_constraint(TEST_TYPE) )
    }

    local function assert_pass(t)
        lunit.assert_true(passes_schema(schema, t)) 
    end

    local function assert_fail(t)
        lunit.assert_false(passes_schema(schema, t)) 
    end

    assert_pass {}
    assert_pass {
        foo = {b = true}
    }
    assert_fail {
        foo = {b = true, c = true}
    }

    assert_pass {
        bar = {d = {}, e = 1, f = "", g = function() end}
    }
    assert_fail {
        bar = {}
    }

    assert_pass {
        baz = {h = {}, i = 1, j = "", k = function() end}
    }
    assert_pass {
        baz = {}
    }

    assert_pass {
        flibby = setmetatable({}, TEST_TYPE)
    }
    assert_fail {
        flibby = {}
    }
    
end