function copy_function_compile_from_attribute_list(func, simple_attributes, recursive_attributes, 
        --[[Optional]] metacopy_attributes, --[[Optional]] prelude, --[[Optional]] body)
    local func_parts = {prelude}
    table.insert(func_parts, "\nreturn function(a, b)\n")
    for attrib in values(simple_attributes) do
        table.insert(func_parts, ("b[%s] = a[%s]\n"):format(attrib, attrib))
    end
    for attrib in values(metacopy_attributes or {}) do
        table.insert(func_parts, ("a[%s]:__copy(b[%s])\n"):format(attrib, attrib))
    end
    table.insert(body)
    table.insert(func_parts, "\nend")
    local func_string = table.concat(func_parts)
    print("COMPILING FUNCTION:")
    print(func_string)
    local func_loader, err = loadstring(func_string)
    if err then error(err) end
    return func_loader()
end
--
---- Create a lazy-optimized copy routine that replaces the given holder+key.
---- It writes the function there, and later overwrites it when the copy is triggered.
--function table.copy_lazy_compile(function_holder, key, is_recursive, is_fixed)
--    local target = function_holder[key]
--    
--end