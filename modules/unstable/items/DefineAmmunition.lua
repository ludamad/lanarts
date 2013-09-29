local ammunition_define,Traits = (import ".ItemDefineUtils").ammunition_define, import ".ItemTraits"

ammunition_define {
    name = "Arrow",
    description = "A mediocore arrow.",
    sprite = "sprites/arrow.png%32x32",
    traits = {Traits.ARROW},
    speed = 6.5
}