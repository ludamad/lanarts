local M = nilprotect {} -- Submodule


--int AttackStats::atk_damage(MTwist& mt, const EffectiveStats& stats) const {
--    const CoreStats& core = stats.core;
--    bool has_projectile = !projectile.empty();
--    WeaponEntry& wentry = weapon.weapon_entry();
--    int dmg = 0;
--
--    if (!has_projectile
--            || is_compatible_projectile(wentry, projectile.projectile_entry())) {
--        const DamageStats& dmgmod = wentry.attack.damage_modifiers;
--        dmg += dmgmod.damage_stats.calculate(mt, core);
--        dmg += round(dmgmod.magic_percentage * stats.magic.damage);
--        dmg += round(dmgmod.physical_percentage * stats.physical.damage);
--    }
--    if (has_projectile) {
--        ProjectileEntry& pentry = projectile.projectile_entry();
--        dmg += projectile.projectile_entry().damage_stats().calculate(mt, core);
--        if (!is_compatible_projectile(wentry, pentry)) {
--            dmg += round(pentry.magic_percentage() * stats.magic.damage);
--            dmg += round(pentry.physical_percentage() * stats.physical.damage);
--        }
--    }
--    return dmg;
--}
--
--int AttackStats::atk_power(MTwist& mt, const EffectiveStats& stats) const {
--    const CoreStats& core = stats.core;
--    bool has_projectile = !projectile.empty();
--    WeaponEntry& wentry = weapon.weapon_entry();
--    int pow = 0;
--
--    if (!has_projectile
--            || is_compatible_projectile(wentry, projectile.projectile_entry())) {
--        WeaponEntry& wentry = weapon.weapon_entry();
--        const DamageStats& dmgmod = wentry.attack.damage_modifiers;
--        pow += dmgmod.power_stats.calculate(mt, core);
--        pow += round(dmgmod.magic_percentage * stats.magic.power);
--        pow += round(dmgmod.physical_percentage * stats.physical.power);
--    }
--    if (has_projectile) {
--        ProjectileEntry& pentry = projectile.projectile_entry();
--        pow += projectile.projectile_entry().power_stats().calculate(mt, core);
--        if (!is_compatible_projectile(wentry, pentry)) {
--            pow += round(pentry.magic_percentage() * stats.magic.power);
--            pow += round(pentry.physical_percentage() * stats.physical.power);
--        }
--    }
--    return pow;
--}
local function attack_power(attack, derived_stats)
    
end

function M.attack_stats_calc(attack, derived_stats)

end

return M