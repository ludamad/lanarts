local StatML = import "@StatML"

local files = io.directory_search(path_resolve "", "*.yaml", --[[recursive]] true)
for _,file in ipairs(files) do
    local testname = file:match("(%d%d_%w*%.yaml)")
    if testname then
        TestCases[testname] = function()
            StatML.reset()
            StatML.load_file(file)
            StatML.parse_all()
            local asserts = StatML.instances.Asserts
            assert(asserts, "No assert instance defined!")
            for k, v in pairs(asserts) do
                assert(v == true, ("Asserts.%s was not 'true', was '%s'!"):format(k, tostring(v)))
            end
        end
    end
end