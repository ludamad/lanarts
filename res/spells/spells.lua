
function attack_all_seen(caster, _, _) 
	local m = monsters_seen(caster)
	local stats = caster.stats
	local pow = 1
	show_message("Doing " .. pow + stats.magic / 4.5 .." with dmg " .. rand_range(4, 10) + stats.magic*2 )
	for k, mon in ipairs(m) do
		mon:damage(pow + stats.magic / 4.5, rand_range(4, 10) + stats.magic*2)
	end

end


powerstrike_spell = {}

function powerstrike_spell.action(caster, _, _) 
    if caster:is_local_player() then 
      show_message("You strike wildly in all directions!", {200,200,255}) 
    elseif caster.name == "Your ally" then
      show_message(caster.name .. " strikes wildly in all directions!", {200,200,255}) 
    end
    caster:add_effect("Charge", 8).callback = 
      function (effect, caster)
        local mons = level.monsters_list()
        for num=1,#mons do
          local mon = mons[num]
          if distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + caster.target_radius + 35 then
            caster:melee(mon)
            local chance = math.max(25, 100 - (num-1) * 20) 
            if rand_range(0, 100) < chance then -- decreasing chance of knockback
            	print(tostring(mon) .. ", " .. tostring(mon.add_effect))
                mon:add_effect("Thrown", 45 + 2 * caster.stats.level).angle = direction({caster.x, caster.y}, {mon.x, mon.y})
                if caster:is_local_player() then 
                  show_message("The " .. mon.name .." is thrown back!", {200,200,255}) 
                end
            end
          end
        end
      end
end