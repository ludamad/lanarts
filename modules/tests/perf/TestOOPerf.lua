local function benchmark(fmt, f)
    local timer = timer_create()
    f()
    print( fmt:format(timer:get_milliseconds()) )
end

local tinsert, tdeep_clone = table.insert, table.deep_clone

local function benchmark_type(desc, T, S)
    local N = 100000
    local objects, copies = {}, {}
    benchmark(desc .. " " .. N .. " Object Creations: %sms", function()
        for i=1,N do
            tinsert(objects,  T.create(S.create(1), S.create(2), S.create(3)))
        end
    end)
    if not desc:match "ffi" then 
        benchmark(desc .. " " .. N .. " Object Clones: %sms", function()
            for i=1,N do
                tinsert(copies, tdeep_clone(objects[i]))
            end
        end)
    end
end

local NT_Simple, NT_Complex = newtype(), newtype()
function NT_Simple:init(v) self.v = v end
function NT_Complex:init(a, b, c)
    self.a,self.b,self.c = a,b,c
end

benchmark_type("newtype", NT_Complex, NT_Simple)

local CL_Simple = { create = function(v) return {v=v} end}
local CL_Complex = { create = function(a, b, c) return {a=a,b=b,c=c} end}

benchmark_type("closures", CL_Complex, CL_Simple)

if not __LUAJIT then
    return
end

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

benchmark_type("ffi (luajit-only)", FFI_Complex, FFI_Simple)