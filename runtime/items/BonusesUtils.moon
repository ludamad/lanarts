GameObject = require "core.GameObject"

ANIMATE_VELOCITIES = {
    {-4, -4}
    {0, -4}
    {4, -4}
    {4, 0}
    {4, 4}
    {0, 4}
    {-4, 4}
    {-4, 0}
}

create_animation = (obj, sprite) =>
    GameObject.animation_create {
        xy: obj.xy
        sprite: sprite
        duration: 25
        velocity: ANIMATE_VELOCITIES[(@n_animations % #ANIMATE_VELOCITIES) + 1]
    }
    -- To make sure animations dont sync, add random jitter:
    @n_animations += if chance 0.1 then 2 else 1

return {:create_animation}
