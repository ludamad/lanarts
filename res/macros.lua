print "Running macros.lua"

local function print_unused_keys(obj, keys) 
    local unused = {}
    for k, v in pairs(obj) do
        local found=false
        for k2 in values(keys) do
            if k2 == k then found=true end
        end
        if not found then
            table.insert(unused, k)
        end
    end
    if #unused > 0 then
        print "UNUSED:"
        for unused in values(unused) do
            print(unused)
        end
    end
end

local function pkv(key, nocomma)
    local parts = {}
    if obj[key] == nil then return false end
    table.insert(parts, '    ')
    table.insert(parts, tostring(key))
    table.insert(parts, " = ")
    table.insert(parts, pretty_tostring(obj[key], 0, true))
    table.insert(parts, nocomma and "" or ",")
    table.insert(printed, key)

    print( table.concat(parts) )
    return true
end

for v in values(armour) do
    if v.equipment_type == "gloves" then
        obj = v
        printed = {}
        print( "Data.equipment_create {")
        pkv("name")
        pkv("description")
        pkv("type")
        print("")
        if pkv("shop_cost") then
            print("")
        end
        if pkv("spr_item") then
            print("")
        end
        if pkv("spell_cooldown_multiplier") then 
            print("")
        end
        if pkv("ranged_cooldown_multiplier") then
            print("")
        end
        pkv("reduction")
        if pkv("magic_reduction") then
            print("")
        end
        pkv("resistance")
        if pkv("magic_resistance") then
            print("")
        end
        pkv("stat_bonuses", true)
        print("}\n")
        print_unused_keys(obj, printed)
    end
end

print "Finished running macros.lua"
io.flush()
