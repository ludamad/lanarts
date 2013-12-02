local function benchmark(fmt, f)
    collectgarbage("collect")
    local timer = timer_create()
    f()
    print( fmt:format(timer:get_milliseconds()) )
end

local rawget,rawset,assert,error=rawget,rawset,assert,error
local tinsert, tdeep_clone = table.insert, table.deep_clone

local function benchmark_type(desc, T, S)
    local N = 500000
    local objects, copies = {}, {}
    benchmark(desc .. " %sms\t" ..N.. " Object Creations", function()
        for i=1,N do
            tinsert(objects,  T.create(S.create(1), S.create(2), S.create(3)))
        end
    end)
    benchmark(desc .. " %sms\t" ..N.. " Object Indexing", function()
        local sum = 0
        local results = {}
        for i=1,N,2 do
            local obj1,obj2 = objects[i],objects[i+1]
            results[i] = obj1.a.v * obj2.a.v + obj1.b.v * obj2.b.v + obj1.c.v * obj2.c.v
            results[i+1] = results[i] -- Prevent holes
        end
    end)
    benchmark(desc .. " %sms\t" ..N.. " Object Mutation", function()
        for i=1,N,2 do
            local obj1,obj2 = objects[i],objects[i+1]
            for j=1,2 do
                obj1.c.v = obj2.a.v * obj2.b.v
                obj2.a.v = obj1.c.v + obj1.b.v
            end
        end
    end)
--    if not desc:match "ffi" then 
--        benchmark(desc .. " %sms\t" ..N.. " Object Clones:", function()
--            for i=1,N do
--                tinsert(copies, tdeep_clone(objects[i]))
--            end
--        end)
--    end
end

function TestCases.OOPerf_newtype_warmup()
    local NT_Simple, NT_Complex = newtype(), newtype()
    function NT_Simple:init(v) self.v = v end
    function NT_Complex:init(a, b, c)
        self.a,self.b,self.c = a,b,c
    end
    
    benchmark_type("newtype_warmup", NT_Complex, NT_Simple)
end

function TestCases.OOPerf_newtype()
    local NT_Simple, NT_Complex = newtype(), newtype()
    function NT_Simple:init(v) self.v = v end
    function NT_Complex:init(a, b, c)
        self.a,self.b,self.c = a,b,c
    end
    
    benchmark_type("newtype", NT_Complex, NT_Simple)
end

function TestCases.OOPerf_simpletable()
    local CL_Simple = { create = function(v) return {v=v} end}
    local CL_Complex = { create = function(a, b, c) return {a=a,b=b,c=c} end}
    
    benchmark_type("simpletable", CL_Complex, CL_Simple)
end

function TestCases.OOPerf_unrolled_ifelsechain()
    local Unrolled_Simple, Unrolled_Complex = newtype(), newtype()
    function Unrolled_Simple:init(v) self[1] = v end
    function Unrolled_Simple:__index(k) if k == "v" then return rawget(self, 1) end end
    function Unrolled_Complex:__newindex(k, v)
        if k == "v" then rawset(self, 1, v) end
        error("No such field")
    end
    function Unrolled_Complex:init(a, b, c) rawset(self,1,a); rawset(self,2,b); rawset(self,3,c) end
    function Unrolled_Complex:__index(k) 
        local idx = (k == "a" and 1) or (k == "b" and 2) or (k == "c" and 3)
        return rawget(self, idx)
    end
    function Unrolled_Complex:__newindex(k, v)
        local idx = (k == "a" and 1) or (k == "b" and 2) or (k == "c" and 3)
        rawset(self, idx, v)
    end
    
    benchmark_type("Unrolled", Unrolled_Complex, Unrolled_Simple)
end

function TestCases.OOPerf_unrolled_indexmap()
    local Unrolled2_Simple, Unrolled2_Complex = newtype(), newtype()
    function Unrolled2_Simple:init(v) rawset(self, 1, v) end
    function Unrolled2_Simple:__index(k) assert(k == "v") return rawget(self, 1) end
    function Unrolled2_Complex:init(a, b, c) rawset(self, 1, a) ; rawset(self, 2, b) ; rawset(self, 3, c) end
    local key_table = {a=1,b=2,c=3}
    function Unrolled2_Complex:__index(k) return rawget(self, rawget(key_table, k)) end
    
    benchmark_type("Unrolled2", Unrolled2_Complex, Unrolled2_Simple)
end

function TestCases.OOPerf_flextypes_table_based()
    local Flex = {}
    typedef(Flex) "Simple" [[v : int]]
    typedef(Flex) "Complex" [[a, b, c : Simple]]

    benchmark_type("Flex_warmup", Flex.Complex, Flex.Simple)
    benchmark_type("Flex", Flex.Complex, Flex.Simple)
end

if not __LUAJIT then
    return
end

function TestCases.OOPerf_ffi()
    local ffi = require("ffi")
    
    ffi.cdef [[
        typedef struct { int v; } FFI_Simple;
        typedef struct { FFI_Simple a, b, c; } FFI_Complex;
    ]]
    
    local function image_ramp_green(n)
        local img = ffi.new "FFI_Complex"
    end
    
    local FFI_Simple = { create = function(v) local self = ffi.new "FFI_Simple" ; self.v = v ; return self  end}
    local FFI_Complex = { create = function(a, b, c) local self = ffi.new "FFI_Complex" ; self.a,self.b,self.c = a,b,c ; return self end}
    
    benchmark_type("ffi warmup (luajit-only)", FFI_Complex, FFI_Simple)
    benchmark_type("ffi (luajit-only)", FFI_Complex, FFI_Simple)
end