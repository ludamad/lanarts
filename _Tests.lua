local Flex = import_copy("@flexparse", "@codegen")
local Struct = import "@Struct"

--local function test_def(def)
--    local s = Struct.create("TestStruct")
--    fp.parse_def(s, def)
--end
--
--function TestCases.test_typedef()
--    test_def("extend T")
--    test_def("F")
--    test_def("F(z)")
--    test_def("F as V")
--    test_def("F(s) as V")
--    test_def("a,b,c : p")
--    test_def("z,a,s : p(1)")
--end

function TestCases.test_typedef()
    local S = Struct.create()
    Flex.parse_def(S, "x, y : int")
    local T = Flex.compile_type(S)
    local p = T.create(1,2)
end