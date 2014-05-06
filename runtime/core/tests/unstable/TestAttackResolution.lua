-- Test the horribly complicated damage-defence resolution function

local AR = import "unstable.AttackResolution"

function TestCases.test_simple_resolution()
    local dmg = AR.resolve_damage(
        --[[Attack damages]] {1},
        --[[Attack defence multipliers]] {{melee = 1}},
        --[[Defence aptitudes]] {melee = 1}
    )
    lunit.assert(dmg == 0)
end

function TestCases.test_complex_attack_resolution()
    lunit.assert_equal(5,
        AR.resolve_damage(
            {15, 15},
            {{melee = 1,  fire = 1}, {melee = 1}},
            {melee = 15, fire = 10}
        )
    )
end