
local function avg(val)
	if not val then 
		return 0
	elseif type(val) == 'table' then
        return (val[1] + val[2]) / 2
    else 
        return val
    end
end

local INTERVAL = 30
local function dmgformula(power, damage, resist, reduction)
    local base_damage = damage - reduction
    if base_damage < 0 then return 0 end
    local pdiff = (power - resist) / INTERVAL
    if pdiff < 0 then
        return base_damage / (1.0 - pdiff)
    else
        return base_damage * (1.0 + pdiff)
    end
end

local function enemy_order(a,b)
	return a[2] < b[2]
end

local function hits_for_mons(power, damage, magicness)
    print("\t\thits_for_mons("..power..","..damage..","..magicness..")")
    local physicalness = 1 - magicness
    local name_and_hits = {}

    for name, mons in pairs(enemies) do
        local stats = mons.stats
        local hp = avg(stats.hp)
        local def = avg(stats.defence)
        local will = avg(stats.willpower)
        local physdam = dmgformula(power, damage, def, avg(stats.physical_reduction))
        local magicdam = dmgformula(power, damage, will, avg(stats.magic_reduction))
        local dam = physdam * physicalness + magicdam * magicness
        local avgdam = 99999
        if dam > 0 then avgdam = hp / dam end
		table.insert(name_and_hits, {name, avgdam})
    end 
    table.sort(name_and_hits, enemy_order)
    
    for _, namedam in ipairs(name_and_hits) do
    	local name, avgdam = unpack(namedam)
    	avgdam = math.floor(avgdam*1000)/1000
        print("\t\t\t"..name .. ": AVG: " .. avgdam .. " hits")
    end
end

local function hitstats(atk, willpower, defence)

end

local function hits_for_player(willpower, defence)
    print("\t\thits_for_player("..power..","..damage..","..magicness..")")
    local physicalness = 1 - magicness
    local name_and_hits = {}

    for name, mons in pairs(enemies) do
        local stats = mons.stats
        local hp = avg(stats.hp)
        local def = avg(stats.defence)
        local will = avg(stats.willpower)
        local physdam = dmgformula(power, damage, def, 0)
        local magicdam = dmgformula(power, damage, will, 0)
        local dam = physdam * physicalness + magicdam * magicness
        local avgdam = 99999
        if dam > 0 then avgdam = hp / dam end
		table.insert(name_and_hits, {name, avgdam})
    end 
    table.sort(name_and_hits, enemy_order)
    
    for _, namedam in ipairs(name_and_hits) do
    	local name, avgdam = unpack(namedam)
        print("\t\t\t"..name .. ": AVG: " .. avgdam .. " hits")
    end
end

local function atlevel(stats, gain, leveln)
	local copy = {}
	for k,v in pairs(stats) do copy[k] = v end
	for i=1, leveln do
		for k,v in pairs(gain) do 
			copy[k] = copy[k] + v 
		end
	end
	return copy
end

function class_test(class, magicness)
	print("Class test for "..class)
	local stats = classes[class].start_stats
	local gain = classes[class].gain_per_level
	for leveln=1,10 do
		local copy = atlevel(stats, gain, leveln)
		print("\tFor level " .. leveln)
		if magicness == 1.0 then
			hits_for_mons(copy.magic + 1.5, copy.magic + 1, magicness)
		else
			hits_for_mons(copy.strength * 1.6 + 12.5, copy.strength *1.1 + 3.5, magicness)
		end
	end
end

function level_tests()
	class_test("Mage", 1.0)
	class_test("Fighter", 0.0)
end